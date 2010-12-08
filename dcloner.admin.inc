<?php
function dcloner_admin_view() {
	return drupal_get_form('system_modules');
}

function dcloner_admin_form() {
    // User related information
    $form['settings'] = array(
        '#type' => 'fieldset',
        '#title' => t('Settings'),
        '#collapsible' => False,
        '#prefix' => '<div class="section corners">'.theme('dcloner_help'),
        '#suffix' => '</div>',
        '#attributes' => array(
            'class' => 'dcloner-info corners'
        ),
    );

    return $form;
}
