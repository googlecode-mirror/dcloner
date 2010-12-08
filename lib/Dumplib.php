<?php
define('lnbr', '\n\r');

if (!function_exists('mysql_dump')) {
    function mysql_dump($database) {
        $query = '';
        $c = mysql_connect('localhost', 'monitor', 'monitor');
        mysql_selectdb($database);
        
        $tables = mysql_list_tables($database);

        while ($row = mysql_fetch_row($tables)) { $table_list[] = $row[0]; }

        for ($i = 0; $i < count($table_list); $i++) {
            $results = mysql_query('SHOW ' . $database . '.' . $table_list[$i]);

            $query .= 'DROP TABLE IF EXISTS `' . $database . '.' . $table_list[$i] . '`;' . lnbr;
            $query .= lnbr . 'CREATE TABLE `' . $database . '.' . $table_list[$i] . '` (' . lnbr;

            $tmp = '';

            while ($row = mysql_fetch_assoc($results)) {
                $query .= '`' . $row['Field'] . '` ' . $row['Type'];

                if ($row['Null'] != 'YES') { $query .= ' NOT NULL'; }
                if ($row['Default'] != '') { $query .= ' DEFAULT \'' . $row['Default'] . '\''; }
                if ($row['Extra']) { $query .= ' ' . strtoupper($row['Extra']); }
                if ($row['Key'] == 'PRI') { $tmp = 'primary key(' . $row['Field'] . ')'; }

                $query .= ','. lnbr;
            }
            
            $query .= $tmp . lnbr . ');' . str_repeat(lnbr, 2);
            
            $results = mysql_query('SELECT * FROM '.$database . '.' . $table_list[$i]);
            
            while ($row = mysql_fetch_assoc($results)) {
                $query .= 'INSERT INTO `' . $database . '.' . $table_list[$i] .'` (';
                $data = Array();
            
                while (list($key, $value) = each($row)) {
                    $data['keys'][] = $key; $data['values'][] = addslashes($value); 
                }

                $query .= join($data['keys'], ', ') . ')' . lnbr . 'VALUES (\'' . join($data['values'], '\', \'') . '\');' . lnbr;
            }

            $query .= str_repeat(lnbr, 2);
        }

        return $query;
    }
}
