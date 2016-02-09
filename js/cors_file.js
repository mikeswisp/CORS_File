/**
 * @file
 * Uploads a Cross Origin File to Rackspace.
 * Adapted from a recipe in the s3fs module
 */

(function ($) {
  RackspaceCF = {};
  Drupal.behaviors.RackspaceCF = {},
  upload_directory = '';

  RackspaceCF.upload_file = function($file_input, button_name) {

    var $form = $file_input.closest('form'),
    $widget = $file_input.parent(),
    file_real = null,
    file_obj = $file_input[0].files[0],
    $progress_message = $('<div id="cors-file-preparing"><p>Uploading File...</p></div>'),
    endpoint_url = '',
    file = file_obj.name.split('.'),
    $progress_bar = $('<div id="progress-bar" class="progress"><div class="progress-bar"></div></div>');

    // Don't do anything if the <input> is empty.
    if (!$file_input.val()) {
      alert('There was no file present. Please add a file and try again');
      return;
    }

    if ($file_input[0].files === undefined || window.FormData === undefined) {
      // If we're in IE8/9, or the FormData API is unavailable, fall back to
      alert('CORS Upload is not supported in IE8 or 9.');
      return;
    }
    // Disable all the submit buttons, so users can't accidentally mess
    // up the workflow. We'll submit the form via JS after file uploads
    // are complete.
    $form.find('input[type="submit"]').attr('disabled', 'disabled');

    // Replace the file input with a bootstrap style progress bar
    $file_input.hide().after($progress_message);
    // Add the actual progress bar
    $file_input.parent().parent().find('.description').prepend($progress_bar);

    // get the upload directory that the file needs to be written to
    var upload_directory = $file_input.parent().find('.upload_location').val();
    // get valid file extensions to validate against on the backend
    var upload_extensions = $file_input.parent().find('.upload_extensions').val();
    // Create the endpoint URL to generate the temporary URL
    endpoint_url = '/endpoint/filename/' + file[0] + '/type/' + file[1] + '/'+ upload_directory + '/' +upload_extensions;

    // POST to drupal to get our temporary URL
    $.post(endpoint_url, null, endpoint_results, 'json');

    // The result, error or success of the POST above
    function endpoint_results(response) {
       if (response.success) {
         file_real = file_obj.name;

         // Send a PUT request to Rackspace.
         // This request contains the file being uploaded
         $.ajax({
            url: response.success,
            type: 'PUT',
            data: file_obj,
            cache: false,
            contentType: false,
            processData: false,
            xhr: RackspaceCF.progress,
            error: function(jqXHR, textStatus, errorThrown) {
              //alert the user that something went wrong
              alert(errorThrown);
              //clean up the form if the error was thrown
              form_cleanup();
            },
            complete: submit_to_drupal
          });

       } else if (response.error){
         alert(response.error);
         form_cleanup();
       }
    }

     // Undoes the form alterations we made.
    function form_cleanup() {
      $file_input.show();
      $progress_message.remove();
      $progress_bar.remove();
      $form.find('input[type="submit"]').removeAttr('disabled');
    }

    // Attaches the needed data on successful response of Rackspace PUT request
    // and submits the form to Drupal
    function submit_to_drupal(data, textStatus, jqXHR) {

      // Update the hidden fields for the file that was just uploaded.
      $widget.find('input.filemime').val(file_obj.type);
      $widget.find('input.filesize').val(file_obj.size);
      $widget.find('input.filename').val(file_real);

      $form.find('input[type="submit"]').removeAttr('disabled');
      var button_id = $widget.find('input.cors-file-form-submit').attr('id');

      var ajax = Drupal.ajax[button_id];
      // Remove the file itself from the form, to avoid sending it to Drupal.
      $(ajax.form[0]).find('#' + $file_input.attr('id')).remove();
      ajax.form.ajaxSubmit(ajax.options);
    }
  };

 /**
  * xhr callback function that updates with the percentage completed
  * idea based upon bootstrap's progress bar
  */
  RackspaceCF.progress = function() {
    var xhr = new window.XMLHttpRequest();
    xhr.upload.addEventListener("progress",
      function(evt) {
        if (!evt.lengthComputable) return;
          var percentComplete = evt.loaded / evt.total;
          var percent = String(100*percentComplete);
          $("#progress-bar div.progress-bar").text('Uploading Progress: '+ percent + '%');
    }, false);
    return xhr;
  };

  /**
   * Implements Drupal.behaviors.
   */
  Drupal.behaviors.RackspaceCF.attach = function(context, settings) {
    
    var ajax_upload_button = $('input.cors-file-form-submit', context);

    // attach one submit handler per each CORS File field on the page
    for (var i = 0; i < ajax_upload_button.length; i++) {
      $(ajax_upload_button[i]).once('cors_file_upload', function(e) {
        $(ajax_upload_button[i]).unbind('mousedown');
        $(ajax_upload_button[i]).one('click', submit_handler);
        
      });
    }

    // The actual sbmit handler function
    function submit_handler(e) {
      e.preventDefault();
      var file_input = $(this).siblings('input.cors-file-upload-file');
      // Make sure we are sending a file to be uploaded
      if (file_input[0].files[0] != undefined) {
        RackspaceCF.upload_file(file_input, 0, ajax_upload_button.attr('name'));
      } else {
        alert('Please upload a file before saving!');
      }
    }
  };
})(jQuery);
