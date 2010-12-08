<?php
class Cloner {
    private $host;
    private $user;
    private $instance;
    private $config;
    private $pass;
    private $ts;
    private $hostname;
    private $full_hostname;

    private $new_host;

    public function __construct(array $host, array $user) 
    {
        $this->hostname = $host['hostname'];

        $host['hostname'] = preg_replace('/\./', '__', $host['hostname']);

        $this->host = $host;
        $this->user = $user;
        $this->pass = substr(md5(mt_rand(0, 1000000)), 0, 15);
        $this->ts = mktime();
    }
    
    public function create(array $opts, array $config, $restart = False) 
    {
        $this->config = $config;
        $this->instance = $opts;

        $this->old_host = $config['path'].'/all';

        if ($this->host['type'] == 'subdomain') {
            $this->full_hostname = $this->hostname.'.'.$_SERVER['HTTP_HOST'];
            $this->new_host = $config['path'].'/'.$this->hostname.'.'.$_SERVER['HTTP_HOST'];
        } else if ($this->host['type'] == 'subdir') {
            $this->full_hostname = $_SERVER['HTTP_HOST'].'.'.$this->hostname;
            $this->new_host = $config['path'].'/'.$_SERVER['HTTP_HOST'].'.'.$this->hostname;
        } else {
            $this->full_hostname = $this->hostname;
            $this->new_host = $config['path'].'/'.$this->hostname;
        }

        $sql = t(
            '
                SELECT clone_id 
                FROM {dclones} 
                WHERE hostname=\'!host\'
            ', 
            array('!host' => $this->hostname.'.'.$_SERVER['HTTP_HOST'])
        );

        $host_exists = db_result(db_query($sql));

        if (isset($host_exists) && intval($host_exists) != 0) {
            drupal_set_message(t('Such subdomain already exists.'));

            drupal_goto('admin/dcloner/list');
        }

        // Hostname (domain name) operations //
        
        // Make dump for a new instance and restore into a new one
        $this->_prepareDump();
        
        // Creates directory structures, links selected modules and themes
        $this->_prepareHost();

        // Add virtualhost configuration
        //$this->_addHostConfig();

        // Makes configuration on new a drupal instance
        $this->_addInstanceConfig();
        $this->_saveSettings();

        if ($restart)
            $this->serverRestart();
    }

    public function update($clone_id = 0)
    {
        $sql = '
            UPDATE {dclones}
            SET server_type=\'!server\',
                status=\'!status\',
                changed=\'!changed\'
                description=\'!description\'
            WHERE clone_id=!clone_id
        ';

        
    }

    private function _prepareDump() 
    {
        global $db_url;

        $creds = parse_url($db_url);
        $ts = mktime();
        $dump_cmd = 'mysql!part -h localhost -u !user -p!pass !db !op '.$this->config['backups'].'/!ts.sql';

        $cmd = t(
            $dump_cmd,
            array(
                '!part' => 'dump',
                '!user' => $creds['user'],
                '!pass' => $creds['pass'],
                '!db' => substr($creds['path'], 1),
                '!ts' => $this->hostname,
                '!op' => '>'
            )
        );

        system($cmd); 

        $c = mysql_connect('localhost', $creds['user'], $creds['pass']);

        $new_db = t(
            '
                CREATE DATABASE `!db` 
                DEFAULT CHARACTER SET utf8 
                COLLATE utf8_bin;
            ',
            array('!db' => $this->host['hostname'])
        );

        $permissions = t(
            '
                GRANT ALL PRIVILEGES ON \'!db\'.* 
                TO \'!user\'@\'localhost\' 
                IDENTIFIED BY \'!pass\'
            ',
            array(
                '!db' => $this->host['hostname'],
                '!user' => $this->host['hostname'],
                '!pass' => $this->host['hostname'].'_'.$this->pass
            )
        );

        mysql_query('SET NAMES UTF-8;', $c);
        mysql_query($new_db, $c);
        mysql_query($permissions, $c);

        $cmd = t(
            $dump_cmd,
            array(
                '!part' => '',
                '!user' => $creds['user'],
                '!pass' => $creds['pass'],
                '!db' => $this->host['hostname'],
                '!ts' => $this->hostname,
                '!op' => '<'
            )
        );

        system($cmd);
    }

    private function _prepareHost() 
    {
        if (!file_exists($this->new_host)) {
            mkdir($this->new_host, 0777);
            mkdir($this->new_host.'/modules');
            mkdir($this->new_host.'/themes');
            mkdir($this->new_host.'/files', 0777);

            //dsm($this->new_host.' ----- '.DOC_ROOT.'sites/www.'.$this->full_hostname);

            symlink($this->new_host, DOC_ROOT.'/sites/www.'.$this->full_hostname);

            if (!file_exists($this->old_host.'/libraries')) {
                mkdir($this->new_host.'/libraries');
            } else {
                symlink($this->old_host.'/libraries', $this->new_host.'/libraries');
            }

            $this->_link('themes');
            $this->_link('modules');
        }
    }

    private function _addHostConfig() 
    {
        $config = DOC_ROOT.'/'.DCLONER.'/tpl/web-servers/'.$this->host['server'].'/virtualhost.conf';
        $vhostconfig = file_get_contents($config);

        $vhost = t(
            $vhostconfig, 
            array(
                '!hostname' => $this->hostname,
                '!doc_root' => DOC_ROOT,
                '!port' => $this->host['port']
            )
        );
        //dsm($this->config);
        $file = sprintf('%s/%s/%s', $this->config['vhosts_dir'], $this->host['server'], $this->hostname);
        //dsm($file);
        $f = fopen($file.'.conf', 'w');

        fwrite($f, $vhost);
        fclose($f);
    }
    
    private function _addInstanceConfig() 
    {
        $config = DOC_ROOT.'/'.DCLONER.'/tpl/config/settings.php';
        $settings = file_get_contents($config);

        $instance_config = t(
            $settings, 
            array(
                '!user' => $this->host['hostname'],
                '!pass' => $this->host['hostname'].'_'.$this->pass,
                '!db' => $this->host['hostname'],
                '!db_prefix' => $this->config['db_prefix'],
            )
        );

        $f = fopen($this->new_host.'/settings.php', 'w');

        fwrite($f, $instance_config);
        fclose($f);
    }
    
    private function _saveSettings() 
    {
        $sql = "INSERT INTO {dclones} VALUES (NULL, '%s', '%s', '%s', '%s', '%s', '%s', '0', '%s')";

        db_query(
            $sql, 
            $this->hostname, 
            $this->host['type'],  
            $this->host['server'],
            $this->host['port'],  
            $this->host['status'],
            $this->ts, 
            $this->host['description']
        );

        $id = db_result(db_query('SELECT LAST_INSERT_ID()'));
        $sql = "INSERT INTO {dclone_users} VALUES (NULL, '%s', '%s', '%s', '%s')";

        db_query(
            $sql, 
            $id, 
            $this->user['firstname'], 
            $this->user['lastname'], 
            $this->user['email']
        );
    }

    private function _link($type) 
    {
        chdir($this->new_host.'/'.$type);

        for ($i = 0; $i < count($this->instance[$type]); $i++) {
            $item = $this->instance[$type][$i];

            if (!file_exists($item))
                symlink($this->old_host.'/'.$type.'/'.$item, $item);
        }
    }
}
