/* -*- indent-tabs-mode: nil; js-indent-level: 2; -*- */
/**
 * @file
 *   Initialize onUnload scripts.
 */
(function($){
  Drupal.behaviors.sheetnode_views_lock = {
    sheetnode_data_views_unlock_onleave: function  (data) {
      data = data || {async: false};
      var lockname = Drupal.settings.sheetnode_data_views_lock.lockname;
      var ajax_key = Drupal.settings.sheetnode_data_views_lock.ajax_key;
      jQuery.ajax({
        url: Drupal.settings.basePath + 'ajax/sheetviews_lock/'+lockname+'/canceledit',
        type: "post",
        data: {k: ajax_key},
        async: false,
        cache: false,
        success: function(e) {
          console.log('unlock through. Good');
        }
      });
    },
    sheetnode_data_views_lock_confirm: function () {
      if (Drupal.settings.sheetnode_data_views_lock.unload_js_message_enable)
        return Drupal.t(Drupal.settings.sheetnode_data_views_lock.unload_js_message);
    },
    attached: false,
    attach: function(context) {
      if (this.attached) {
        return ;
      }
      this.attached = true;
      if (!Drupal.settings.sheetnode_data_views_lock) {
        return ;
      }
      /* Prevent submitting the node form from being interpreted as "leaving the page" */
      $(Drupal.settings.sheetnode_data_views_lock.internal_forms).submit(function () {
        userMovingWithinSite();
      });

      // Looks like onUserExit is having a bug
      $().onUserExit( {
        execute: Drupal.behaviors.sheetnode_views_lock.sheetnode_data_views_unlock_onleave,
        executeConfirm: Drupal.behaviors.sheetnode_views_lock.sheetnode_data_views_lock_confirm,
        internalURLs: Drupal.settings.sheetnode_data_views_lock.internal_urls
      });

      // Ensure unlock is called
      $(window).on('beforeunload', function(e){
        // Confirm message
        e.returnValue = null;
        if (movingWithinSite == false) {
          e.returnValue = Drupal.behaviors.sheetnode_views_lock.sheetnode_data_views_lock_confirm();
        }

        return e.returnValue;
      });
      $(window).on('unload', function(e){
        if(/Firefox[\/\s](\d+)/.test(navigator.userAgent) && new Number(RegExp.$1) >= 4) {
           var data={async:false};
           Drupal.behaviors.sheetnode_views_lock.sheetnode_data_views_unlock_onleave(data);
        }
        else {
           var data={async:true};
           Drupal.behaviors.sheetnode_views_lock.sheetnode_data_views_unlock_onleave(data);
        }
        return null;
      });

      //make lock request to ensure current views locked
      if (window.location.pathname.indexOf("admin/structure/views") == -1) {
        var lockname = Drupal.settings.sheetnode_data_views_lock.lockname;
        jQuery.ajax({
          url: "https://pacificstair.com/ajax/sheetviews_lock/"+ lockname+ "/lock",
          async: false,
          cache: false,
          success: function(data) {
            response = data.pop();
            if (response && response.settings && response.settings.sheetnode_data_views_lock) {
              Drupal.settings.sheetnode_data_views_lock = Drupal.settings.sheetnode_data_views_lock || {};
              Drupal.settings.sheetnode_data_views_lock.lockname = response.settings.sheetnode_data_views_lock.lockname;
              Drupal.settings.sheetnode_data_views_lock.ajax_key = response.settings.sheetnode_data_views_lock.ajax_key;
            }
          }
        });
      }
    }
  };

})(jQuery);
