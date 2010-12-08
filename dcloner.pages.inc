<?php
include 'lib/Cloner.class.php';

function dcloner_create_instance() {
	return drupal_get_form('dcloner_create_form');
}

function dcloner_list_instance($short = False) {
	$sql = '
	   SELECT 
	       c.clone_id, c.hostname, c.server_type, c.status, c.description, c.type,
	       cu.uid, cu.firstname, cu.lastname, cu.email
       FROM {dclones} c
       JOIN {dclone_users} cu
       ON c.clone_id = cu.clone_id
       ORDER BY c.status DESC;
    ';

    $insances = array();
    $insances_short = array();

    $header_long = array(
        '#',
        t('Hostname'),
        t('Server'),
        t('User'),
        t('Status'),
        t('Action'),
    );

    $header_short = array(
        t('Hostname'),
        t('Server'),
        'data'=> array('data' => t('Generate configuration'), 'class' => 'text-center')
    );

    $req = db_query($sql);

    $cnt = 0;

    $statuses = array(
        'enabled' => t('Active'),
        'disabled' => t('Disabled'),
        'deleted' => t('Disabled'),
    );
    
    $servers = array(
        'apache' => 'Apache',
        'nginx' => 'Nginx'
    );

    while ($row = db_fetch_array($req)) {
        $cnt++;

        $hostname = 'http://';
        $link = ''; 
        $state = 'instance-active';

        if ($row['type'] == 'subdomain')
            $hostname .= preg_replace('/_/', '.', $row['hostname']).'.'.$_SERVER['HTTP_HOST'];
        else if ($row['type'] == 'subdir')
            $hostname .= $_SERVER['HTTP_HOST'].'/'.preg_replace('/_/', '.', $row['hostname']);
        else
          $hostname .= preg_replace('/_/', '.', $row['hostname']);

        if ($row['status'] !== 'enabled') 
            $state = 'instance-disabled';

        $id = $row['clone_id'];
        $link = l($hostname, $hostname, array('attributes' => array('target' => '_blank')));

        $edit = t(
            '<div class="action-edit">'.
            l(
                '<img src="/'.DCLONER.'/images/edit.png" />', 
                'admin/dcloner/edit/'.$row['clone_id'],
                array(
                    'html' => True,
                    'attributes' => array(
                        'title' => t('Edit instance')
                    )
                )
            ).
            '</div>'
        );

        $remove = t(
            '<div class="action-delete">'.
            l(
                '<img src="/'.DCLONER.'/images/delete.png" />', 
                'admin/dcloner/delete/'.$row['clone_id'],
                array(
                    'html' => True,
                    'attributes' => array(
                        'title' => t('Delete instance')
                    )
                )
            ).
            '</div>'
        );

        $test_view = t(
            '<div class="action-view">'.
            l(
                '<img src="/'.DCLONER.'/images/test-view.png" />', 
                'admin/dcloner/view/'.$row['clone_id'],
                array(
                    'html' => True,
                    'attributes' => array(
                        'class' => 'action-view-link',
                        'title' => t('View instance !i', array('!i' => $hostname)),
                        'rel' => $hostname,
                        'target' => '_new'
                    )
                )
            ).
            '</div>'
        );

        $host = array(
            'data' => $link, 
            'title' => t('Go to: ').$hostname,
        );

        $server = array(
            'data' => $servers[$row['server_type']], 
            'class' => 'clone-server '.$row['server_type'], 
            'title' => $servers[$row['server_type']],
        );

        $user = array(
            'data' => $row['firstname'].' '.$row['lastname'], 
            'class' => 'user-data', 
            'title' => t('Owner\'s name.'),
        );

        $status = array(
            'data' => $statuses[$row['status']], 
            'class' => 'clone-status '.$state, 
            'title' => $statuses[$row['status']],
        );

        $actions = array(
            'data' => $edit.$remove.$test_view, 
            'class' => 'clone-actions',
            'title' => t('Actions'),
        );

        $insances[] = array(
            'data' => array(
                array('data' => $cnt, 'class' => 'count'),
                $host,
                $server,
                $user,
                $status,
                $actions,
            )
        );
        
        $conf = t(
            '<div class="dcloner-generate">!a | !n | !l | !i</div>', 
            array(
                '!a' => '<span class="server-apache">'.l(t('Apache'), 'admin/dcloner/generate/'.$id.'/apache').'</span>',
                '!n' => '<span class="server-nginx">'.l(t('Nginx'), 'admin/dcloner/generate/'.$id.'/nginx').'</span>',
                '!l' => '<span class="server-lhttpd">'.l(t('LightHttpd'), 'admin/dcloner/generate/'.$id.'/lhttpd').'</span>',
                '!i' => '<span class="server-iis">'.l(t('IIS'), 'admin/dcloner/generate/'.$id.'/iis').'</span>'
            )
        );
        
        $generate = array(
            'data' => $conf,
            'class' => 'generate-config',
            'title' => t('Config post generation')
        );

        $insances_short[] = array(
            'data' => array(
                $host,
                $server,
                $generate
            )
        );
    }

    drupal_add_js(DCLONER.'/js/tooltip.js');
    drupal_add_js(DCLONER.'/vendor/dragndrop/js/jquery.dragndrop.js');

    if (count($insances) != 0 && !$short) {
        return theme('table', $header_long, $insances, array('class' => 'instances-list'));
    } else if ($short) {
        return theme('table', $header_short, $insances_short, array('class' => 'instances-short-list'));
    }

    return t('No instance found');
}

function dcloner_edit_instance($instance_id) {
	return drupal_get_form('dcloner_edit_form');
}

function dcloner_delete_instance($id = 'None') {
	return drupal_get_form('dcloner_delete_confirm_form');
}

function dcloner_freeze_instance() {
	return 'I CAN FREEZE DRUPAL INSTANCES';
}

function dcloner_delete_host($id = 0) {
    if ($id === 0)
        return;

    $sql = t(
        '
            SELECT * 
            FROM {dclones} 
            WHERE clone_id = !id 
            LIMIT 1
        ', 
        array('!id' => $id)
    );
    
    $delete_clone = t(
        '
            DELETE FROM {dclones} 
            WHERE clone_id = !id 
            LIMIT 1
        ', 
        array('!id' => $id)
    );

    $delete_clone_user = t(
        '
            DELETE FROM {dclone_users} 
            WHERE clone_id = !id 
            LIMIT 1
        ', 
        array('!id' => $id)
    );

    $query = db_query($sql);
    $host = db_fetch_array($query);
    
    if (count($host) < 1) 
        drupal_set_message(t('Such subdomain does not exist.'));

    if (db_query($delete_clone) && db_query($delete_clone_user)) 
        drupal_set_message(t('%host successfully removed.', array('%host' => $host['hostname'])));

    drupal_goto('admin/dcloner/list');
}

function check_field($name = '', $val = '') {
    if (!isset($val)) {
        form_set_error($name, t('Please fill empty fields.'));
    }

    return true;
}

function read_dir($dir = '', $dirs_only = False, $files_only = False) {
    $files = array();

    if ($handle = opendir($dir)) {
        while (false !== ($file = readdir($handle))) {
            if ($file != "." && $file != ".." && substr($file, 0, 1) !== '.') {
                if ($dirs_only) {
                    if (is_dir($dir.'/'.$file) && $file !== 'dcloner') {
                        $files[] = $file;
                    } else { 
                        continue;
                    }
                }

                if ($files_only) {
                    if (is_file($dir.'/'.$file)) {
                        $files[] = $file;
                    } else { 
                        continue;
                    }
                }

                if (!$dirs_only && !$files_only) {
                    $files[] = $file;
                }
            }
        }

        closedir($handle);
    }

    return $files;
}

function get_info($type = '', $file = '') {
    $files = array();
    $return = array();
    $info_file = '';
    $path = '';

    if ('module' === $type) {
        $path = DRUPAL_MODULES.'/'.$file;
    } elseif ('theme' === $type) {
        $path = DRUPAL_THEMES.'/'.$file;
    }

    $files = read_dir($path, False, True);

    for ($i = 0; $i < count($files); $i++) {
        if (preg_match('/\.info/', $files[$i])) {
            if (!file_exists($path.'/'.$module.'/'.$files[$i]))
                continue;

            $data = file($path.'/'.$module.'/'.$files[$i]);

            if (count($data) <= 0)
                continue;

            for ($j = 0; $j < count($data); $j++) {
                if (preg_match('/=/', $data[$j])) {
                    $kv = preg_split('/=/', $data[$j]);                    

                    $return[trim($kv[0])] = trim($kv[1]);
                }
            }
        }
    }

    return $return;
}

function dcloner_build_my_list($dir) {
    $items = read_dir($dir, True);
    $type = '';
    $name = '';

    if (DRUPAL_MODULES === $dir) {
        $type = 'module';

        $table_header = array(
            '?',
            t('Module'),
            t('Descripion'),
        );
    } else if (DRUPAL_THEMES === $dir) {
        $type = 'theme';

        $table_header = array(
            '?',
            t('Theme'),
            t('Descripion'),
        );
    }

    $rows = array();

    //dsm($items);

    for ($i = 0; $i < count($items); $i++) {
        $info = get_info($type, $items[$i]);

        if (count($info) <= 0)
            continue;

        $check_box = theme('dcloner_checkbox', $type.'s', $items[$i]);

        $name = theme('dcloner_make_label', $items[$i], $info['name']);
        $host = 'http://'.$_SERVER['HTTP_HOST'];
        $path = '/sites/all/themes/';

        if ($type == 'theme') {
            if (file_exists(DRUPAL_THEMES.'/'.$items[$i].'/screenshot.png'))
                $screenshot = $host.$path.$items[$i].'/screenshot.png';
            
            if (file_exists(DRUPAL_THEMES.'/'.$items[$i].'/images/screenshot.png'))
                $screenshot = $host.$path.$items[$i].'/images/screenshot.png';

            $name .= '<img src="'.$screenshot.'" />';
        }

        $rows[] = array(
            'data' => array(
                array('data' => $check_box),
                array('data' => $name),
                array('data' => $info['description']),
            ),
        );
    }
    
    return theme('table', $table_header, $rows);
}

function dcloner_summary() {
    $sql = "SELECT COUNT(*) FROM {dclones} WHERE status='%s'";
    $sql_warnings = "SELECT hostname, type FROM {dclones} WHERE status='deleted'";
    $sql_notifictions = "";    
    $warnings = array();
    $notifictions = array();

    drupal_add_js(DCLONER.'/js/admin-controls.js');

    $q = db_query($sql_warnings);

    while ($hostname = db_fetch_array($q)) {
        if (file_exists(DOC_ROOT.'/'.$hostname['hostname']))
            $warnings[] = t(
                'Deleted %instance Debaser Instance still exists.', 
                array('%instance' => $hostname['hostname'])
            );

            $notifictions[] = t(
                'Please remove %instance sites folder and database as well if needed.', 
                array('%instance' => $hostname['hostname'])
            );
    }

    if (!is_writable(DOC_ROOT.'/sites'));
        $warnings[] = t('Directory %sites is not writable.', array('%sites' => 'sites'));

    $stat = array(
        'enabled' => db_result(db_query($sql, 'enabled')),
        'disabled' => db_result(db_query($sql, 'disabled'))
    );
    
    if (count($warnings) != 0) {
        $stat['warnings'] = theme('item_list', $warnings);
    } else {
        $stat['warnings'] = t('There are no warnings here');
    }

    if (count($notifictions) != 0) {
        $stat['notifications'] = theme('item_list', $notifictions);
    } else {
        $stat['notifications'] = t('Nothing here...');
    }

    $stat['warnings_cnt'] = count($warnings);
    $stat['notifications_cnt'] = count($notifictions);

    //drupal_get_form('system_modules');
    return theme('dcloner_summary', $stat);

    //dsm(module_list($refresh = False, $bootstrap = True, $sort = False));
    //theme('dcloner_summary', $enabled_clones, $disabled_clones);
}

function dcloner_create_form() {
    global $db_url;
    global $db_prefix;
    global $base_path;
    
    $host = $_SERVER['HTTP_HOST'];

    // Virtual host information
    $form['host_info'] = array(
        '#type' => 'item',
        '#title' => t('Virtual host'),
        '#collapsible' => True,
        '#collapsed' => True,
        '#prefix' => '<div class="section corners host-info box">',
        '#suffix' => '</div>',
        '#attributes' => array(
            'class' => 'dcloner-info corners'
        ),
    );

    $form['host_info']['info'] = array(
        '#type' => 'item',
        '#value' => t('<span style="color: #EE1111; font-size: 11px;">Fill only subdomain or subdirectory\'s name you want to create without www.</span>'),
    );

    $form['host_info']['hostname'] = array(
        '#type' => 'textfield',
        '#title' => t('Hostname'),
        '#value' => '',
        '#size' => 15,
        '#maxlength' => 40,
        '#required' => True,
        '#description' => t('Example: example will look like example.%h', array('%h' => $_SERVER['HTTP_HOST'])),
        '#prefix' => '<div class="dcloner-text-element">',
        '#suffix' => '</div>',
    );

    $form['host_info']['port'] = array(
        '#type' => 'textfield',
        '#title' => t('Port'),
        '#value' => '',
        '#size' => 4,
        '#maxlength' => 6,
        '#value' => 80,
        '#required' => True,
        '#description' => t('Port number'),
    );

    $form['host_info']['type'] = array(
        '#type' => 'radios',
        '#title' => t('Clone type'),
        '#default_value' => 'subdomain',
        '#required' => True,
        '#options' => array(
            'subdomain' => t('www.<strong><big>sub<big></strong>.!h', array('!h' => $host)),
            'subdir' => t('www.!h/<strong><big>sub</big></strong>', array('!h' => $host)), 
            'other' => t('Any other domain/site.')
        ),
        '#description' => t('Sample: type 1 - www.sub.example.com, type 2 - www.example.com/sub'),
        '#prefix' => '<div class="domain-type">',
        '#suffix' => '</div>'
    );

    $form['host_info']['servers'] = array(
        '#type' => 'radios',
        '#title' => t('Web server'),
        '#default_value' => 'apache',
        '#options' => array(
            'apache' => t('Apache.'), 
            'nginx' => t('Nginx.')
        ),
        '#prefix' => '<div class="server">',
        '#suffix' => '</div>',
    );

    // User related information    
    $form['user_info'] = array(
        '#type' => 'item',
        '#title' => t('User info'),
        '#collapsible' => False,
        '#collapsed' => False,
        '#prefix' => '<div class="section corners user-info box">',
        '#suffix' => '</div>',
        '#attributes' => array(
            'class' => 'dcloner-info corners'
        ),
    );

    $form['user_info']['name'] = array(
        '#type' => 'textfield',
        '#title' => t('Name'),
        '#value' => '',
        '#size' => 15,
        '#maxlength' => 40,
        '#description' => t('Instance owner\'s name. Maximum 40 alphabetic symbols.'),
        '#prefix' => '<div class="dcloner-text-element">',
        '#suffix' => '</div>',
    );

    $form['user_info']['lastname'] = array(
        '#type' => 'textfield',
        '#title' => t('Lastname'),
        '#value' => '',
        '#size' => 15,
        '#maxlength' => 40,
        '#description' => t('Instance owner\'s lastname. Maximum 40 alphabetic symbols.'),
        '#prefix' => '<div class="dcloner-text-element">',
        '#suffix' => '</div>',
    );

    $form['user_info']['email'] = array(
        '#type' => 'textfield',
        '#title' => t('Email'),
        '#value' => '',
        '#size' => 15,
        '#maxlength' => 40,
        '#description' => t('Instance owner\'s email. Maximum 40 alphabetic symbols.'),
        '#prefix' => '<div class="dcloner-text-element">',
        '#suffix' => '</div>',
        '#element_validate' => array('valid_email_address'),
    );

    // Cloning options
    $form['clone_info'] = array(
        '#type' => 'item',
        '#title' => t('Drupal options'),
        '#collapsible' => True,
        '#collapsed' => False,
        '#prefix' => '<div class="section corners drupal-info">',
        '#suffix' => '</div>',
        '#attributes' => array(
            'class' => 'dcloner-info corners'
        ),
    );

    $form['clone_info']['drupal'] = array(
        '#type' => 'item',
        '#title' => t('Allowed modules and themes'),
        '#prefix' => '<div class="dcloner-label">',
        '#suffix' => '</div>',
    );

    $form['clone_info']['modules'] = array(
        '#type' => 'fieldset',
        '#title' => t('Select modules'),
        '#collapsible' => True,
        '#collapsed' => True,
    );

    $form['clone_info']['modules']['list'] = array(
        '#type' => 'item',
        '#value' => dcloner_build_my_list(DRUPAL_MODULES),
        '#prefix' => '<div class="dcloner-label">',
        '#suffix' => '</div>',
    );

    $form['clone_info']['themes'] = array(
        '#type' => 'fieldset',
        '#title' => t('Select themes'),
        '#collapsible' => True,
        '#collapsed' => True,
    );
    
    $form['clone_info']['themes']['list'] = array(
        '#type' => 'item',
        '#value' => dcloner_build_my_list(DRUPAL_THEMES),
        '#prefix' => '<div class="dcloner-label">',
        '#suffix' => '</div>',
    );

    $form['clone_info']['description'] = array(
        '#type' => 'textarea',
        '#title' => t('Short description'),
        '#default_value' => t('Just another copy...'),
        '#required' => False,
        '#prefix' => '<div class="dcloner-text-element">',
        '#suffix' => '</div>',
        '#rows' => 2,
        '#cols' => 50
    );

    // Controls
    $form['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Create'), 
        '#prefix' => '<div class="section corners dcloner-form-controls">',
        '#attributes' => array(
            'class' => 'corners dcloner-form-button'
        ),
    );

    $form['cancel'] = array(
        '#type' => 'submit',
        '#value' => t('Cancel'), 
        '#suffix' => '</div>',
        '#attributes' => array(
            'class' => 'corners dcloner-form-button'
        ),
    );

    return $form;
}

function dcloner_create_form_validate(array $form) {
    global $db_url;

    $sql = t(
        'SELECT COUNT(hostname) FROM {dclones} WHERE hostname="!h" LIMIT 1', 
        array('!h' => $post['hostname'])
    );

    $post = $form['#post'];

    check_field('name', $post['name']);
    check_field('lastname', $post['lastname']);
    check_field('email', $post['email']);
    check_field('port', $post['port']);
    check_field('servers', $post['servers']);
    check_field('hostname', $post['hostname']);
    
    if (intval(db_result(db_query($sql)), INT_CAST) == 1) {
        form_set_error('hostname', t('Such hostname already exists, please pick another name'));
    }
}

function dcloner_create_form_submit(array $form) {
    global $db_prefix;

    $post = $form['#post'];
    $settings = parse_ini_file('config.conf');

    $host = array(
        'hostname' => $post['hostname'],
        'type' => $post['type'],
        'port' => $post['port'],
        'server' => $post['servers'],
        'status' => 'enabled',
        'description' => $post['description']
    );

    $user = array(
        'firstname' => $post['name'],
        'lastname' => $post['lastname'],
        'email' => $post['email'],
    );

    $opts = array(
        'themes' => $post['themes'],
        'modules' => $post['modules'],
    );

    $cfg = array(
        'path' => DOC_ROOT.'/sites',
        'vhosts_dir' => $settings['virtualhosts'],
        'backups' => $settings['backups'],
        'db_prefix' => $db_prefix
    );

    $cloner = new Cloner($host, $user);
    $cloner->create($opts, $cfg);
}

function dcloner_delete_confirm_form() {
    $form = array();
    $sql = 'SELECT hostname FROM {dclones} WHERE clone_id = %d';
    $instance_id = arg(3);

    $result = db_result(db_query($sql, $instance_id));

    $form['delete'] = array(
        '#type' => 'item',
        '#title' => t('Do you really want to delete !host?', array('!host' => $result)),
    );

    $form['delete']['id'] = array(
        '#type' => 'hidden',
        '#value' => $instance_id,
    );

    $form['delete']['split'] = array(
        '#type' => 'item',
        '#prefix' => '<hr />',
    );

    $form['delete']['confirm'] = array(
        '#type' => 'submit',
        '#value' => t('Yes'),
    );

    $form['delete']['cancel'] = array(
        '#type' => 'submit',
        '#value' => t('No'),
    );

    return $form;
}

function dcloner_dumper_form() {
    $props = '" width="100%" height="462" frameborder="0" style="margin:0;">';
    $src = ' src="'.$base_url.'/sxd/?druid='.session_name();

    return array(
        'clone_info' => array(
            'advanced' => array(
                '#type' => 'item',
                '#prefix' => '<br><iframe'.$src.$props,
                '#suffix' => '</iframe>'
            )
        )
    );
}

function dcloner_delete_confirm_form_submit(array $form) {
    if ($form['#post']['op'] === 'Yes') {
        dcloner_delete_host($form['#post']['id']);

        drupal_set_message('Host successfully deleted');
    }

    drupal_goto('admin/dcloner/list');
}

function dcloner_edit_form() {
    $form = dcloner_create_form();

    $sql = t(
        '
            SELECT 
                c.clone_id, c.hostname, c.type, c.server_type, c.port, c.status, c.description,
                cu.uid, cu.firstname, cu.lastname, cu.email
            FROM {dclones} c
            JOIN {dclone_users} cu
            ON c.clone_id = cu.clone_id
            AND c.clone_id = !clone_id LIMIT 1;
        ',
        array('!clone_id' => arg(3))
    );

    $instance = db_fetch_array(db_query($sql));

    $form['user_info']['name']['#value'] = $instance['firstname'];
    $form['user_info']['lastname']['#value'] = $instance['lastname'];
    $form['user_info']['email']['#value'] = $instance['email'];
   
    $form['host_info']['hostname']['#value'] = $instance['hostname'];
    $form['host_info']['hostname']['#disabled'] = True;
    $form['host_info']['hostname']['#description'] = t('You cannot edit hostname.');
    $form['host_info']['port']['#value'] = $instance['port'];
    $form['host_info']['type']['#default_value'] = $instance['type'];
    $form['host_info']['servers']['#default_value'] = $instance['server_type'];

    unset($form['host_info']['type']);
    unset($form['host_info']['servers']);

    $form['clone_info']['description']['#value'] = $instance['description'];

    $form['clone_info']['clone_id'] = array(
        '#type' => 'hidden',
        '#default_value' => $instance['clone_id']
    );

    return $form;
}

function dcloner_edit_form_validate($form) {
    dcloner_create_form_validate($form);
}

// TODO
function dcloner_edit_form_submit($form) {
    dsm($form);
}

function dcloner_makedump() {
    return drupal_get_form('dcloner_dumper_form');
}
