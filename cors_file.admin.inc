<?php

/**
 * @file
 * Administration settings to authenticate Rackspace Cloud Files
 * with Drupal
 *
 * @see Related modules
 * - Cloud Files
 *
 */


/**---------------------------------------------------------
 *
 * @Admin hooks and utility functionality
 *
 *---------------------------------------------------------*/

/**
 * Simple utility function for getting .
 *
 * Taken directly from the cloud_files module
 *
 * @return array
 *   Rackspace Cloud authentication URLs
 */
function _cors_file_rackspce_auth_urls() {

  $info = libraries_load('php-opencloud');
  if ($info['loaded']) {
    return array(
     'us' => array(
        'name' => t('United States'),
        'url' => OpenCloud\Rackspace::US_IDENTITY_ENDPOINT,
      ),
      'gb' => array(
        'name' => t('United Kingdom'),
        'url' => OpenCloud\Rackspace::UK_IDENTITY_ENDPOINT,
      ),
    );
  }
  else {
    form_set_error('', t('PHP OpenStack library could not be loaded.'));
  }
}


/**
 * Simple utility function for getting Rackspace Cloud Regions.
 *
 * Taken directly from the cloud_files module
 *
 * @return array
 *   Rackspace Cloud Regions
 */
function _cors_file_rackspace_regions() {

  return array(
    'DFW' => t('Dallas'),
    'ORD' => t('Chicago'),
    'LON' => t('London'),
  );
}


/**
 * Function that creates a form for an admin user to maintain
 * their Rackspace cloud container attached to the site.
 *
 * Based upon a recipe in cloud files
 * @link https://www.drupal.org/project/cloud_files
 */
function cors_file_admin() {

  $authentication_urls = _cors_file_rackspce_auth_urls();
  // authentication URL
  $form['rackspace_auth_url'] = array(
    '#type' => 'item',
    '#title' => t('Authentication URL'),
  );
  // The Rackspace authentication URLs
  foreach ($authentication_urls as $key => $item) {
    $form['rackspace_auth_url'][$key] = array(
      '#type' => 'radio',
      '#title' => $item['name'] . ' ( ' . $item['url'] . ' ) ',
      '#default_value' => variable_get('rackspace_auth_url'),
      '#return_value' => $item['url'],
      '#parents' => array('rackspace_auth_url'),
    );
  }

  $form['rackspace_region'] = array(
    '#type' => 'item',
    '#title' => t('Container Region'),
  );

  $container_regions = _cors_file_rackspace_regions();
  // The Rackspace region we are dealing with
  foreach ($container_regions as $key => $name) {
    $form['rackspace_region'][$key] = array(
      '#type' => 'radio',
      '#title' => $name . ' ( ' . $key . ' ) ',
      '#default_value' => variable_get('rackspace_region'),
      '#return_value' => $key,
      '#parents' => array('rackspace_region'),
    );
  }
  // Rackspace username
  $form['rackspace_username'] = array(
    '#type' => 'textfield',
    '#title' => t('Username'),
    '#description' => t('Your Rackspace API username'),
    '#default_value' => variable_get('rackspace_username'),
    '#required' => TRUE,
  );
  // Rackspace API key
  $form['rackspace_api_key'] = array(
    '#type' => 'password',
    '#title' => t('API Key'),
    '#description' => t('Your Rackspace API key'),
    '#default_value' => variable_get('rackspace_api_key'),
  );
  // The Container
  $form['rackspace_container'] = array(
    '#type' => 'textfield',
    '#title' => t('Container'),
    '#description' => t('The target Container you are uploading file to'),
    '#default_value' => variable_get('rackspace_container'),
    '#required' => TRUE,
  );
  // The Container's domain
  $form['rackspace_domain'] = array(
    '#type' => 'textfield',
    '#title' => t('Target (Domain)'),
    '#description' => t('Found under "View Website Settings," or as the base URL of a file'),
    '#default_value' => variable_get('rackspace_domain'),
    '#required' => TRUE,
  );

  $form['rackspace_debug'] = array(
    '#type' => 'checkbox',
    '#title' => t('Debug'),
    '#description' => t('Check the box to enable logging.'),
    '#default_value' => variable_get('rackspace_debug'),
  );

  return system_settings_form($form);
}

/**
 * Validation function for the form in cors_file_admin()
 *
 * @param $form
 *  The form array being validated
 *
 * @param &$form_state
 *  A refernce to the current state of the form that includes submitted values
 *
 */
 function cors_file_admin_validate($form, &$form_state) {

  $info = libraries_load('php-opencloud');
  if ($info['loaded']) {
    //the validation key is used to set the correct field with the correct error
    $validation_key = '';
    //url with validation
    $url = $form_state['values']['rackspace_auth_url'];
    if (empty($url) || $url == NULL) {
      form_set_error(
        $form['rackspace_auth_url'],
        t('Make sure you have selected an authentication URL')
      );
      return;
    }
    //region with validation
    $region = $form_state['values']['rackspace_region'];
    if (empty($region) || $region == NULL) {
      form_set_error(
        'rackspace_region',
        t('Make sure you have selected a region')
      );
      return;
    }
    // username
    $username = $form_state['values']['rackspace_username'];
    // api key
    $api_key = $form_state['values']['rackspace_api_key'];
    if(empty($api_key)) $api_key = variable_get('rackspace_api_key');
    // container
    $container_name = $form_state['values']['rackspace_container'];
    // domain
    $domain = $form_state['values']['rackspace_domain'];
    // debug
    $debug = $form_state['values']['rackspace_debug'];

    try {
      //set the error key to the api key
      $validation_key = 'rackspace_api_key';
      $client = new OpenCloud\Rackspace($url,
        array(
         'username' => $username,
         'apiKey' => $api_key,
        )
      );
      //http://docs.php-opencloud.com/en/latest/services/object-store/index.html
      //authenticate using the RackSpace setup
      $client->authenticate();
      //set the error key to the region
      $validation_key = 'rackspace_region';
      //Now to instantiate the Object Store service
      $service = $client->objectStoreService(
        NULL,
        $region,
        'publicURL'
      );
      //set the error key to the container
      $validation_key = 'rackspace_container';
      //attempt to get the container
      //if this fails then wen know that the container was invalid
      $container = $service->getContainer($container_name);

      //at this point we are optimistically setting our variables
      //we can do this because we are assuming that an error would be
      //thrown otherwise and this code would not execute
      variable_set('rackspace_username', $username);
      variable_set('rackspace_api_key', $api_key);
      variable_set('rackspace_url', $url);
      variable_set('rackspace_region', $region);
      variable_set('rackspace_domain', $domain);
      variable_set('rackspace_container', $container_name);
      variable_set('rackspace_debug', $debug);

      _cors_file_log_events('Successfully authenticated with Rackspace');
    } catch (Exception $e) {
      _cors_file_log_events($e->getMessage());
      form_set_error($validation_key, $e->getMessage());
      return;
    }
  }
 }
