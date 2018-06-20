/**
 * @file
 *   Initialize user locking interface when click on buttons.
 */
(function($){
  Drupal.behaviors.sheetviews_button_lock = {
    attach: function(context) {
      /* allow user to submit sheetnode (disabled button in PHP)*/
      $('.view .sheetview').on('sheetnodeReady', function(spreadsheet) {
        $(".views-form .form-actions #edit-submit").removeClass('form-button-disabled').attr('disabled', false);
        //prevent double click: disable submit button
        $('form').preventDoubleSubmission();
      });
      //add spinner
      $(".views-form .form-actions input, .view-filters .form-submit").each( function(){
          $(this).click(function(){
            $('body').prepend('<div class="spinner-wrapper"><div class="spinner"><div class="rect1"></div><div class="rect2"></div><div class="rect3"></div><div class="rect4"></div><div class="rect5"></div></div></div>');
            setTimeout(function(){
              $('.spinner-wrapper').remove();
            }, 30000);
          });
        }
      );
    }
  };
  
  
  // jQuery plugin to prevent double submission of forms
  jQuery.fn.preventDoubleSubmission = function() {
    $(this).on('submit',function(e){
      var $form = $(this);

      if ($form.data('submitted') === true) {
        // Previously submitted - don't submit again
        console.log("prevent resubmit same form again");
        e.preventDefault();
      } else {
        // Mark it so that the next submit can be ignored
        $form.data('submitted', true);
      }
    });

    // Keep chainability
    return this;
  };
})(jQuery);
