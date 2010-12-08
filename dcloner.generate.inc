<?php
function dcloner_generate_virtualhost($id = -1, $server = 'apache') {
    if ($id == -1) {
        drupal_goto('admin/dcloner');
    }

    $sql = t('SELECT * FROM {dclones} WHERE clone_id=!id LIMIT 1', array('!id' => $id));

    $result = db_fetch_array(db_query($sql));

    $vhost_config = file_get_contents(DCLONER.'/tpl/web-servers/'.$server.'/virtualhost.conf');

    $vhost_config = t(
        $vhost_config, 
        array(
            '!port' => $result['port'],
            '!hostname' => $result['hostname'],
            '!doc_root' => DOC_ROOT
        )
    );

    return drupal_get_form('dcloner_generate_vhost_form', $vhost_config, $result['hostname']);
}

function dcloner_generate_vhost_form($form_state, $contents, $host) {
    $form['virtualhost'] = array(
        '#type' => 'textarea',
        '#title' => t('Virtualhost configuration settings on !h', array('!h' => $host)),
        '#rows' => 30,
        '#value' => $contents
    );
    
    return $form;
}