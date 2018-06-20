<?php
define("SHEET_LOCKABLE", 10);
define("SHEET_IS_LOCKED", 11);
define("SHEET_IS_LOCKED_BY_CURRENT",12);
define("SHEET_NOT_LOCKABLE",13);
/**
 * Implement of hook_socialcalc_save_alter
 */
function sheetnode_data_socialcalc_save_alter(&$sc) {
  if (current_views_is_locked()) {
    do_the_hard_lock($sc);
  }
}

function do_the_lock(&$form, &$form_state) {
  unset($form['actions']);
}

function do_the_hard_lock(&$sc) {
  foreach ($sc['sheet']['cells'] as &$cell) {
    if (is_array($cell)) {
      $cell['readonly'] = TRUE;
    }
  }
}

function current_views_is_locked() {
  $views = views_get_current_view();
  if (!empty($views) && !empty($views->name)) {
    $lock = sheetnode_data_views_lock_fetch_lock(sheetnode_data_get_lockname($views->name));
    global $user;
    return !empty($lock) && ($lock->session != $user->sid);
  }

  return FALSE;
}
/*
 * get current lock status for $lockname
 * return lock object or FALSE on failure
 */
function sheetnode_data_views_lock_fetch_lock($lockname) {
  $query = db_select('sheetnode_views_data_lock', 'c')
      ->fields('c')
      ->condition('c.views_name', $lockname);
  $u = $query->leftJoin('users', 'u', 'u.uid = c.uid');
  $query->fields($u, array('name'));
  return $query->execute()->fetchObject();
}
/*
 * init sheetviews lock callback
 */
function init_sheetview_lock(&$form, &$form_state, $form_id) {
  global $user;
  $view = $form_state['build_info']['args'][0];
  $status = _get_sheetviews_lock_status($view);
  switch ($status) {
    case SHEET_NOT_LOCKABLE:
      //people who dont have access to check out sheetviews
      do_the_lock($form, $form_state);
      break;
    case SHEET_LOCKABLE:
      if (!empty($_GET['jobnumber'])) {
        // Views is not locked. Try to lock
        if (sheetnode_data_lock_views(sheetnode_data_get_lockname($view->name), $user->uid, $user->sid)) {
          // Adding cancel button, if configured
          if (variable_get('sheetviews_lock_admin_cancelbutton', true)) {
            _sheetnode_data_views_lock_add_cancelbutton($form, $form_state, $form_id);
          }
          if (variable_get('sheetnode_data_lock_unload_js', TRUE)) {
            $form['#after_build'][] = '_sheetnode_data_views_lock_add_unload_js';
          }
        } else {
          drupal_set_message(t('Failed to lock this sheet.'), 'Error');
        }
      }
      break;
    case SHEET_IS_LOCKED:
      // Another people lock this sheet.
      $lock = sheetnode_data_views_lock_fetch_lock(sheetnode_data_get_lockname($view->name));
      drupal_set_message(t('This sheet is locked by @name <br />You can edit this sheet after the user has finished.', array('@name' => $lock->name)), 'status', TRUE);
      do_the_lock($form, $form_state);
      break;
    case SHEET_IS_LOCKED_BY_CURRENT:
      // You lock this sheet on another session.
      // Adding cancel button, if configured
      if (variable_get('sheetviews_lock_admin_cancelbutton', true)) {
        _sheetnode_data_views_lock_add_cancelbutton($form, $form_state, $form_id);
      }
      if (variable_get('sheetnode_data_lock_unload_js', TRUE)) {
        $form['#after_build'][] = '_sheetnode_data_views_lock_add_unload_js';
      }
      /* $url = 'admin/config/content/sheetnode/' . sheetnode_data_get_lockname($view->name) . '/sheetviews_lock/releaseown';
      drupal_set_message(t('This is sheet locked by you. If you closed previous opened sheet, click !here to check back in now.', array('!here' => l(t('here'), $url, array('query' => array('destination' => $_GET['q']))))), 'status', TRUE);
      do_the_lock($form, $form_state); */
      break;
    default:
      break;
  }
}
function sheetnode_data_get_lockname($views_name, $jobnumber = '') {
  if ($jobnumber == '' && !empty($_GET['jobnumber'])) {
    $jobnumber = check_plain($_GET['jobnumber']);
    return implode('_', array($views_name, $jobnumber));
  }
  
}
/*
 * return current lock status
 * 
 */
function _get_sheetviews_lock_status($view) {
  global $user;
  $lock = sheetnode_data_views_lock_fetch_lock(sheetnode_data_get_lockname($view->name));
  if (!user_access('check out sheetviews') || !_sheetnode_data_is_lockable_views($view)) {
    return SHEET_NOT_LOCKABLE;
  }
  elseif ($lock === FALSE) {
    return SHEET_LOCKABLE;
  }
  else if ($lock->uid != $user->uid) {
    return SHEET_IS_LOCKED;
  } 
  elseif ($lock->uid == $user->uid) {
    return SHEET_IS_LOCKED_BY_CURRENT;
  }
}
