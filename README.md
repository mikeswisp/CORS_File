# CORS_File
Drupal 7 module that uploads a file directly to Rackspace Cloud Files and keeps the file managed through Drupal.

## Installation
* Download and install the Drupal [Libraries Module](https://www.drupal.org/project/libraries).  

* Create a folder in your Drupal site for Libraries found at: sites/all/libraries

* Download the [php-opencloud SDK](http://php-opencloud.com/) and install it into your libraries folder so your libraries folder looks like this: sites/all/libraries/php-opencloud.

* Install Composer and require php-opencloud as a dependency.  The finished installion should look like this: sites/all/libraries/php-opencloud/vendor/autoload.php
```php
# Install Composer
curl -sS https://getcomposer.org/installer | php

# Require php-opencloud as a dependency
php composer.phar require rackspace/php-opencloud
```

*  Download and install the Drupal [jQuery Update Module](https://www.drupal.org/project/jquery_update)
*  Make sure you are at least using jQuery 1.5 or greater.

## Requirements
*  Rackspace account, with an API key.
*  At least one Cloud Files Container with public access.
*  PHP 5.4.
*  jQuery 1.5+
*  IE 10+

## Known Limitations
* Currently, the field only supports one file being uploaded at a time.
* All uploaded files are stored in the base directory of the container and preset directories are not yet supported.

## Credits
Recipes, ideas, and functions for this module are based upon the files module, file.inc, stream_wrappers.inc, and the following contrib modules:
* [Cloud Files](https://www.drupal.org/project/cloud_files)
* [S3 File System CORS Upload](https://www.drupal.org/project/s3fs_cors)
* [Remote Stream Wrapper](https://www.drupal.org/project/remote_stream_wrapper)


