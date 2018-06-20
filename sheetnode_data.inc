<?php

function sheetnode_data_views_list() {
  $views_list = array();
  $views = views_get_all_views();

  foreach ($views as $view) {
    if (!empty($view->human_name) && empty($view->disabled)) {
      foreach ($view->display as $k => $v) {
        if ($v->display_options['style_plugin'] == 'sheet') {
          $views_list[$view->name] = $view->human_name;
          break;
        }
      }
    }
  }
  return $views_list;
}
