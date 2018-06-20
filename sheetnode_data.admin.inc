<?php

/*
 * sheetnode data settings form
 */

function sheetnode_data_settings_form($form, &$form_state) {
  $views_list = sheetnode_data_views_list();
  $form['sheetnode_data_views'] = array(
    '#title' => t('Sheetnode data views'),
    '#type' => 'checkboxes',
    '#description' => t('Check views which will using sheetnode data'),
    '#options' => $views_list,
    '#default_value' => variable_get('sheetnode_data_views'),
  );
  $form['sheetnode_data_auto_update'] = array('#title' => t('Auto get node id for specific fields'), '#type' => 'fieldset');
  $edit = array();
  foreach ($views_list as $name => $human_name) {
    $edit['sheetnode_data_auto_update_' . $name] = array('#title' => t($human_name), '#type' => 'textfield', '#description' => t('Please input structure as: search_node_type|search_datatable_field|search_node_field|datatable_update_field'), '#default_value' => variable_get('sheetnode_data_auto_update_' . $name, ''),);
  } $form['sheetnode_data_auto_update']['settings'] = $edit;
  $form = system_settings_form($form);
  $form['#submit'][] = 'sheetnode_data_settings_form_submit';
  return $form;
}

function sheetnode_data_settings_form_submit() {
  // DELETE from `fqi_sheetnode_view` WHERE view_name="pm_logistics"
}


/**
 * Providing an administration interface for sheetnode_data.
 */
function sheetnode_data_views_lock_admin_settings() {
  module_load_include('inc', 'sheetnode_data', 'sheetnode_data.sheetviews');
  $form = array();
  $views_list = sheetnode_data_views_list();

  $form['sheetnode_data_views_lockable'] = array(
    '#title' => t('Sheetnode views will be locked when editing'),
    '#type' => 'checkboxes',
    '#description' => t('Check views which will be locked during editing'),
    '#options' => $views_list,
    '#default_value' => variable_get('sheetnode_data_views_lockable'),
  );
  $form['sheetnode_data_views_lock_cancel_url'] = array(
    '#title' => t('Sheetnode views edit cancel url'),
    '#type' => 'textfield',
    '#description' => t('Default url which sheetviews will be redirect user to when he click cancel'),
    '#default_value' => variable_get('sheetnode_data_views_lock_cancel_url', '<front>'),
  );
  $form['#attached'] = array(
    'js' => array(
      drupal_get_path('module','sheetnode_data') . '/js/sheetnode_data.admin.js',
    ),
  );

  $form['sheetnode_data_lock_unload_js'] = array(
      '#type' => 'checkbox',
      '#title' => t('Use javascript to detect leaving the sheetviews form'),
      '#description' => t('If you disable this, sheetviews will remain locked if the user navigates away from the node without saving it or clicking the cancel button.'),
      '#default_value' => variable_get('sheetnode_data_lock_unload_js', true),
  );
  $form['sheetnode_data_unload_js_fieldset'] = array(
    '#type' => 'fieldset',
    '#title' => t('Javascript detection options.'),
  );
  $form['sheetnode_data_unload_js_fieldset']['sheetnode_data_unload_js_message_enable'] = array(
    '#type' => 'checkbox',
    '#title' => t('Confirm that the user really wants to leave the node form with a Javascript popup'),
    '#description' => t('This will cause a Javascript popup to ask the user if navigating away from the node edit page is intentional, giving the user an opportunity to cancel navigation so that the changes to the node may be saved. This will not appear if the user clicks the cancel button.'),
    '#default_value' => variable_get('sheetnode_data_unload_js_message_enable', TRUE),
  );
  $form['sheetnode_data_unload_js_fieldset']['sheetnode_data_unload_js_message'] = array(
      '#type' => 'textfield',
      '#title' => t('Javascript popup message text'),
      '#description' => t('This option allows you to modify the text the user sees when navigating away from an edited node.'),
      '#default_value' => variable_get('sheetnode_data_unload_js_message', 'If you proceed, ALL of your changes will be lost.'),
  );

  $form['sheetnode_data_admin_verbose'] = array(
      '#type' => 'checkbox',
      '#title' => t('Show lock / unlock messages'),
      '#description' => t('This option controls whether or not sheetnode_data inform the user that he has locked/unlocked a sheetnode. This does not control the javascript dialogues.'),
      '#default_value' => variable_get('sheetnode_data_admin_verbose', true),
  );

  $form['sheetnode_data_admin_cancelbutton'] = array(
      '#type' => 'checkbox',
      '#title' => t('Add cancel button'),
      '#description' => t('Adds a cancel button to the sheetnode form. This way a user can properly cancel the transaction.'),
      '#default_value' => variable_get('sheetnode_data_admin_cancelbutton', true),
  );

  return system_settings_form($form);
}
