<?php

/**
 * Build an overview of locked sheetviews.
 *
 * @param $account
 *   A user object.
 */

function sheetnode_data_views_lock_overview($account = NULL) {
  global $user;
  // TODO: old checkout code, review
  $header = array(array('data' => t('Views_name'), 'field' => 'c.views_name', 'sort' => 'asc'));
  // in the case of an admin, we dont have uid, as he sees all locks
  if (!$account) {
    $header[] = array('data' => t('Username'), 'field' => 'u.name');
    $uid = NULL;
  }
  // otherwise we have the account of the user just beeing views as argument
  else {
    $uid = $account->uid;
  }
  $header[] = array('data' => t('Locked since'), 'field' => 'c.timestamp');
  if($uid == $user->uid || user_access('administer checked out sheetviews')) {
    $header[] = t('Operations');
  }

  $query = db_select('sheetnode_views_data_lock', 'c')
    ->extend('TableSort')
    ->fields('c');
  $u = $query->join('users', 'u', 'u.uid = c.uid');
  $query->fields($u, array('name'));
  if ($uid)
    $query->condition('c.uid', $uid);
  $query->orderByHeader($header);
  $rows = array();
  foreach ($query->execute() as $data) {
    $url = $uid ? "admin/config/content/sheetnode/".$data->views_name."/sheetviews_lock/releaseown" : 'admin/config/content/sheetnode/sheetviews_lock/release/'.$data->views_name;
    $row = array();
    $row[] = t($data->views_name);
    if (!$uid) {
      $row[] = theme('username', array('account' => user_load($data->uid)));
    }
    $row[] = format_date($data->timestamp, 'small');
    if($uid == $user->uid || user_access('administer checked out sheetviews')) {
      $row[] = l(t('release lock'), $url);
    }
    $rows[] = $row;
  }

  $output = theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => 'sheetviews_lock'),
                                 'empty' => t('No locked sheetviews.')));
  $output .= theme('pager', array('quantity' => 50));

  return $output;
}

/**
 * Menu callback; release a locked node for all users or a specific user.
 *
 * @param $views_name
 *   A views name.
 * @param $account
 *   A user object. If passed, the lock will only be released if this
 *   user owned it.
 * @return
 *   This function will execute a redirect and doesn't return.
 */
function sheetnode_data_views_lock_release_item($lockname, $account = NULL) {
  global $user;
  if (!$account && _sheetnode_data_views_lock_verbose()) {
    /*
     * Enable our "lock released" message to inform the user who
     * likely owned the lock which is to be broken.
     */
    $lock = sheetnode_data_views_lock_fetch_lock($lockname);
  }
  sheetnode_data_views_lock_release($lockname, $account ? $account->uid : NULL);
  if(_sheetnode_data_views_lock_verbose()) {
    if (!empty($lock) && !$account && $user->uid != $lock->uid) {
      $lock_account = user_load($lock->uid);
      drupal_set_message(t('The editing lock held by !user has been released.', array('!user' => theme('username', array('account' => $lock_account)))), 'status', FALSE);
    } else {
      drupal_set_message(t('The editing lock has been released.'),'status', FALSE);
    }
  }
  drupal_goto($account ? "user/{$account->uid}/sheetviews_lock" : 'admin/config/content/sheetnode/sheetviews_lock_overview');
}

/**
 * Release the lock of a node. We are using the current users uid, so the user only can delete
 * his own locks. We never fail, as if the lock does not exist, the node is unlocked anyway
 *
 * @param $response
 *   When set to FALSE, indicates that the request was made through
 *   ajax. This means that we shouldn't talk to the user. It also
 *   means that we should compare the ajax_key to fix the page Reload
 *   bug (http://drupal.org/node/1049708). In the page reload bug, the
 *   browser sends a request to load the edit page and simultaneously
 *   sends an AJAX request asking for the node to be unlocked. By
 *   changing the ajax_key when responding to the browser, we can
 *   detect that the soon-to-come ajax request is from the previous
 *   page load and that it should be ignored.
 */
function sheetnode_data_views_lock_release_own_item($lockname, $response = TRUE) {
  global $user;
  if($lockname != NULL) {
    /*
     * Imply that this is an AJAX request if we aren't expected to
     * interface with a human.
     */
    if (!$response) {
      $lock = sheetnode_data_views_lock_fetch_lock($lockname);
      if (isset($lock->ajax_key) && isset($_GET['k']) && strcmp($_GET['k'], $lock->ajax_key)) {
        /* the key doesn't match, don't unlock the node */
        if ($response) {
          drupal_set_message('Trounced AJAX unlock request.', 'status', FALSE);
        }
        exit();
      }
    }
    sheetnode_data_views_lock_release($lockname,$user->uid);
   // drupal_get_messages();
    if($response) {
      drupal_goto(variable_get('sheetnode_data_views_lock_cancel_url', '<front>'));
    }
    else {
      exit();
    }
  }
  else {
    if($response) {
      drupal_goto();
    }
    else {
      exit();
    }
  }
}

/**
 * AJAX callback to lock a node manually.
 *
 * @param object $views
 *   The views to lock.
 */
function sheetnode_data_ajax_lock_callback($lockname) {
  global $user;
  $views_name = explode('_', $lockname);
  $jobnumber = array_pop($views_name); //remove jobnumber
  $views_name = implode('_', $views_name); //rebuild views name
  sheetnode_data_lock_views($lockname, $user->uid, $user->sid, TRUE, $jobnumber);
  //return settings for unlock views
  if ($lock = sheetnode_data_views_lock_fetch_lock($lockname)) {
    $settings['sheetnode_data_views_lock'] = array(
      'lockname' => $lock->views_name,
      'ajax_key' => $lock->ajax_key,
    );
  }
  // Add the javascript that unlocks the node when the user navigates away
  // from the page.
  $form = array('views' => array('#value' => $views_name));
  $form_state['build_info']['args'][0] = views_get_view($views_name);
  _sheetnode_data_views_lock_add_unload_js($form, $form_state);
  $commands = array();
  $commands[] = ajax_command_remove('div.messages');
  $commands[] = ajax_command_before('#block-system-main', theme('status_messages'));
  $commands[] = ajax_command_settings($settings, TRUE);
  ajax_deliver(array('#type' => 'ajax', '#commands' => $commands));
}

/**
 * Try to lock a sheet views for editing.
 *
 * @param $views_name
 *   A views name.
 * @param $uid
 *   The user id to lock the node for.
 * @param $quiet
 *   Suppress any normal user messages.
 * @return
 *   FALSE, if a document has already been locked by someone else.
 */
function sheetnode_data_lock_views($lockname, $uid, $session, $quiet = FALSE, $jobnumber = '') {
  $lock = sheetnode_data_views_lock_fetch_lock($lockname);
  //our new logic require a jobnumber to lock sheetview
  if (empty($_GET['jobnumber']) && empty($jobnumber)) {
    drupal_set_message(t("Can't lock sheetview without jobnumber"), 'error', FALSE);
    return FALSE;
  }
  if ($lock != FALSE && $lock->uid != $uid) {
    $message = sheetnode_data_views_data_lock_owner($lock);
    if (user_access('administer checked out sheetviews')) {
      $url = "admin/config/content/sheetnode/sheetviews_lock/release/$lockname";
    }

    if (isset($url)) {
      $message .= '<br />'. t('Click !here to check back in now.', array('!here' => l(t('here'), $url, array('query' => array('destination' => $_GET['q'])))));
    }
    if(!empty($message)) {
      drupal_set_message($message, 'warning', FALSE);
    }
    return FALSE;
  }
  else {
    // no lock yet, create one
    if($lock == false) {
      // Lock node.
      $data = array(
        'views_name' => $lockname,
        'uid' => $uid,
        'timestamp' => time(),
        'ajax_key' => rand(),
        'session' => $session,
      );
      drupal_write_record(
        'sheetnode_views_data_lock',
        $data
      );

      if(_sheetnode_data_views_lock_verbose() && !$quiet) {
        drupal_set_message(t('This document is now locked against simultaneous editing. It will unlock when you navigate elsewhere.'), 'status', FALSE);
      }
      module_invoke_all('sheetviews_lock_locked', $lockname, $uid);
    } else {
      /* A lock already exists: update its AJAX key */
      $lock->ajax_key = rand();
      if (!drupal_write_record('sheetnode_views_data_lock', $lock, array('views_name'))) {
        /*
         * we encountered a race condition where the lock was deleted
         * between when we loaded it and when we tried to update it
         * with a new key. Recreate the lock then:
         */
        drupal_write_record('sheetnode_views_data_lock', $lock);
      }
    }
  }

  return TRUE;
}

/**
 * Release a locked node.
 *
 * @param $views_name
 *   The views name to release the edit lock for.
 * @param $uid
 *   If set, verify that a lock belongs to this user prior to release.
 */
function sheetnode_data_views_lock_release($lockname, $uid = NULL) {
  $query = db_delete('sheetnode_views_data_lock')
    ->condition('views_name', $lockname);
  if (!empty($uid))
    $query->condition('views_name', $lockname);
  $query->execute();
  module_invoke_all('sheetnode_data_views_lock_released', $lockname);
}

function _sheetviews_lock_release_all_user_locks($uid) {
  db_delete('sheetnode_views_data_lock')
    ->condition('uid', $uid)
    ->execute();
}

/**
 * Check whether a node is configured to be protected by sheetviews_lock.
 */
function _sheetnode_data_is_lockable_views($views) {
  return !in_array(FALSE, module_invoke_all('sheetviews_lock_views_lockable', $views));
}

function _sheetnode_data_views_lock_verbose() {
  return variable_get('sheetnode_data_admin_verbose', true);
}

/**
 * Tell who has locked the sheet
 */
function sheetnode_data_views_data_lock_owner($lock) {
  $username = theme('username', array('account' => user_load($lock->uid)));
  $date = format_date($lock->timestamp, 'medium');
  return t('for editing by !name since @date.', array('!name' => $username, '@date' => $date));
}

function _sheetnode_data_views_lock_add_unload_js(&$form, $form_state) {
  $m = drupal_get_path('module','sheetnode_data');
  drupal_add_js("$m/js/jquery.url.packed.js", array('group' => JS_LIBRARY));
  drupal_add_js("$m/js/onUserExit.js", array('group' => JS_LIBRARY));
  drupal_add_js("$m/js/sheetviews_lock_init.js");
  $views = $form_state['build_info']['args'][0];
  $internal_urls = array('admin/structure/views');
  $internal_form_selectors = array();
  if (!empty($views->display_handler->options['path'])) {
    $internal_urls[] = $views->display_handler->options['path'];
  }
  $internal_form_selectors[] = '.views-exposed-form';
  $lock = sheetnode_data_views_lock_fetch_lock(sheetnode_data_get_lockname($views->name));
  $settings = array(
    'lockname' => sheetnode_data_get_lockname($views->name),
    'ajax_key' => isset($lock->ajax_key) ? $lock->ajax_key : '',
    'unload_js_message_enable' => variable_get('sheetnode_data_unload_js_message_enable', TRUE),
    'internal_urls' => implode('|', $internal_urls),
    'internal_forms' => implode(', ', $internal_form_selectors),
  );
  if ($settings['unload_js_message_enable']) {
    $settings['unload_js_message'] = variable_get('sheetnode_data_unload_js_message', 'If you proceed, ALL of your changes will be lost.');
  }
  /*
   * Workaround for http://drupal.org/node/1525784 where this function
   * is called multiple times when doing a file field AJAX upload and
   * array_merge_recursive() is used instead of
   * drupal_array_merge_deep_array() to construct the Drupal.settings
   * value. Not calling drupal_add_js() multiple times deprives
   * file_ajax_upload() of the ability to mess up here ;-).
   */
  $called =& drupal_static(__FUNCTION__ . '__called');
  if (!empty($called)) {
    $called++;
    return $form;
  }
  $called = 1;

  drupal_add_js(array('sheetnode_data_views_lock' => $settings), 'setting');
  return $form;
}

function _sheetnode_data_views_lock_add_cancelbutton(&$form, $form_state, $form_id) {
  if (!empty($form_state['build_info']['args'][0]->name)) {
    $form['actions']['cancel'] = array(
      '#type' => 'button',
      '#weight' => 2000,
      '#value' => t('Exit'),
      '#validate' => array('sheetnode_data_views_lock_cancel_submit'),
    );
  }
}
