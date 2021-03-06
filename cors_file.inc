<?php
/**
 * @file
 * Implement DrupalStreamWrapperInterface to provide an rackspace:// streamwrapper.
 *
 * @see Adopted from the following related modules and core files
 *  - Cloud Files
 *  - stream_wrappers.inc
 *  - Remote Stream Wrapper
 */

class RackspaceStreamWrapper implements DrupalStreamWrapperInterface {
  /**
   * Instance URI (stream).
   *
   * These streams will be referenced as 'rackspace://target'
   *
   * @var String
   */
  protected $uri;

  /**
   * A generic resource handle.
   */
  public $handle = NULL;

  /**
   * Pointer to where we are in a directory read.
   */
  protected $directoryPointer = 0;

  /**
   * List of files in a given directory.
   */
  protected $directoryList;

  /**
   * The pointer to the next read
   */
  protected $stream_pointer = 0;

  /**
   * A buffer for reading.
   */
  protected $stream_data = NULL;

  /**
   * Object cache
   *
   * @var CFResponse
   */
  protected $object = NULL;

  /**
   * Container cache
   */
  protected $container = NULL;


  /**
   * Implements setUri()
   *
   * @param $uri
   *   A string containing the URI that should be used for this instance.
   */
  public function setUri($uri) {

    $this->uri = $uri;
  }

  /**
   * Implements getUri()
   *
   * @return
   *   Returns the current URI of the instance.
   */
  public function getUri() {

    return $this->uri;
  }

  /**
   * Returns the target of the resource within the stream.
   *
   * @param string $uri
   *   Optional URI.
   *
   * @return string
   *   Returns a string representing a filename of the target.
   */
  protected function getTarget($uri = NULL) {

    if (variable_get('rackspace_debug')) {
      // Add testing variables here for debugging purposes
      $this->writeWatchdog('Func: ' . __FUNCTION__ . ' Line: ' .__LINE__ . ' URI ' . $uri);
    }
    if (!isset($uri)) {
      $uri = $this->uri;
    }
    list($scheme, $target) = explode('://', $uri, 2);
    return trim($target, '\/');
  }

  /**
   * Base implementation of getMimeType().
   */
  public static function getMimeType($uri, $mapping = NULL) {

    if (variable_get('rackspace_debug')) {
      watchdog(
        "cors_file",
        'Func: ' . __FUNCTION__ . ' Line: ' .__LINE__ . ' URI ' . $uri,
        array(),
        WATCHDOG_DEBUG
      );
    }
    if (!isset($mapping)) {
      // The default file map, defined in file.mimetypes.inc is quite big.
      // We only load it when necessary.
      include_once DRUPAL_ROOT . '/includes/file.mimetypes.inc';
      $mapping = file_mimetype_mapping();
    }

    $extension = '';
    $file_parts = explode('.', basename($uri));

    // Remove the first part: a full filename should not match an extension.
    array_shift($file_parts);

    /* Iterate over the file parts, trying to find a match.
     * For my.awesome.image.jpeg, we try:
     *    - jpeg
     *    - image.jpeg, and
     *    - awesome.image.jpeg
     */
    while ($additional_part = array_pop($file_parts)) {
      $extension = strtolower($additional_part . ($extension ? '.' . $extension : ''));
      if (isset($mapping['extensions'][$extension])) {
        return $mapping['mimetypes'][$mapping['extensions'][$extension]];
      }
    }
    return 'application/octet-stream';
  }

  /**
   * Returns a web accessible URL for the resource.
   *
   * @return
   *   Returns a string containing a web accessible URL for the resource.
   */
  public function getExternalUrl() {

    if (variable_get('rackspace_debug')) {
      // Add testing variables here for debugging purposes
      $this->writeWatchdog('Func: ' . __FUNCTION__ . ' Line: ' .__LINE__);
    }
    $target = file_uri_target($this->uri);
    $domain = variable_get('rackspace_domain');

    // Check for http or https in the domain
    // If the protocol is not found add http://
    if (strpos($domain, 'http://') !== FALSE) {
      $url = $domain . '/' . $target;
    } else if (strpos($domain, 'https://') !== FALSE) {
      $url = $domain . '/' . $target;
    } else {
      $url = 'http://' . $domain . '/' . $target;
    }
    return $url;
  }

  /**
   * chmod() is not currently being used
   *
   * @see StreamWrapperInterface
   *
   *
   * @param $mode
   *   Specifies access restrictions
   *
   * @return bool
   */
  public function chmod($mode) {

    if (variable_get('rackspace_debug')) {
      // Add testing variables here for debugging purposes
      $this->writeWatchdog('Func: ' . __FUNCTION__ . ' Line: ' .__LINE__ . ' MODE ' . $mode);
    }
    return TRUE;
  }

  /**
   * Implements realpath()
   *
   * @return string
   *  Returns a path name with the stream wrapper reference
   */
  public function realpath() {

    if (variable_get('rackspace_debug')) {
      // Add testing variables here for debugging purposes
      $this->writeWatchdog('Func: ' . __FUNCTION__ . ' Line: ' .__LINE__);
    }
    return 'rackspace://' . $this->getLocalPath();
  }

  /**
   * Returns the canonical absolute path of the URI, if possible.
   *
   * @param string $uri
   *   (optional) The stream wrapper URI to be converted to a canonical
   *   absolute path. This may point to a directory or another type of file.
   *
   * @return string
   *   Returns the target of the URI.
   */
  protected function getLocalPath($uri = NULL) {

    if (variable_get('rackspace_debug')) {
      // Add testing variables here for debugging purposes
      $this->writeWatchdog('Func: ' . __FUNCTION__ . ' Line: ' .__LINE__ . ' URI ' . $uri);
    }
    if (!isset($uri)) {
      $uri = $this->uri;
    }
    return $this->getTarget($uri);
  }

  /**
   * Opens a stream, as for fopen(), file_get_contents(), file_put_contents().
   *
   * stream_open() is not currently being used
   *
   * @see StreamWrapperInterface
   *
   * @param string $uri
   *   A string containing the URI to the file to open.
   * @param string $mode
   *   The file mode ("r", "wb" etc.).
   * @param $options
   *   A bit mask of STREAM_USE_PATH and STREAM_REPORT_ERRORS.
   * @param string &$opened_path
   *   A string containing the path actually opened.
   *
   * @return bool
   *   Returns TRUE if file was opened successfully. (Always returns TRUE).
   *
   */
  public function stream_open($uri, $mode, $options, &$opened_path) {

    if (variable_get('rackspace_debug')) {
      // Add testing variables here for debugging purposes
      $this->writeWatchdog('Func: ' . __FUNCTION__ . ' Line: ' .__LINE__ . ' URI ' . $uri);
    }
    return TRUE;
  }

  /**
   * Support for flock()
   *
   * stream_lock() is not currently being used
   *
   * @see StreamWrapperInterface
   *
   * @param $operation
   *   One of the following:
   *   - LOCK_SH to acquire a shared lock (reader).
   *   - LOCK_EX to acquire an exclusive lock (writer).
   *   - LOCK_UN to release a lock (shared or exclusive).
   *   - LOCK_NB if you don't want flock() to block while locking (not
   *     supported on Windows).
   *
   * @return bool
   *   Always returns TRUE at the present time. (no support)
   */
  public function stream_lock($operation) {

    if (variable_get('rackspace_debug')) {
      // Add testing variables here for debugging purposes
      $this->writeWatchdog('Func: ' . __FUNCTION__ . ' Line: ' .__LINE__ . ' OPERATION ' . $operation);
    }
    return FALSE;
  }

  /**
   * Support for fread(), file_get_contents() etc.
   *
   * @see StreamWrapperInterface
   *
   * @param int $count
   *   Maximum number of bytes to be read.
   *
   * @return string
   *   The string that was read, or FALSE in case of an error.
   */
  public function stream_read($count) {

    if (variable_get('rackspace_debug')) {
      // Add testing variables here for debugging purposes
      $this->writeWatchdog('Func: ' . __FUNCTION__ . ' Line: ' .__LINE__ );
    }
    // If the data is empty, get the data from the object.
    // This stores all the data in the $streamData property, only request once.
    if (!$this->stream_data) {
      // If the object is empty, get the object from Rackspace.
      if (!$this->object) {
        if (!isset($this->container)) {
          try {
            $this->setContainer();
          } catch (Exception $e) {
            $this->writeWatchdog('Func: ' . __FUNCTION__ . ' Line: ' .__LINE__ . ' URI ' . $e->getMessage());
          }
        }
        $this->object = $this->container->dataObject($target);
      }
      $this->stream_data = $this->object->getContent();
    }
    $data = substr($this->stream_data, $this->stream_pointer, $count);
    $this->stream_pointer += $count;
    return $data;
  }

  /**
   * Support for fwrite(), file_put_contents() etc.
   *
   * stream_write() is not currently being used
   *
   * @see StreamWrapperInterface
   *
   * @param string $data
   *   The string to be written.
   *
   * @return int
   *   The number of bytes written (integer).
   */
  public function stream_write($data) {

    if (variable_get('rackspace_debug')) {
      // Add testing variables here for debugging purposes
      $this->writeWatchdog('Func: ' . __FUNCTION__ . ' Line: ' .__LINE__);
    }
    return FALSE;
  }

  /**
   * Support for feof()
   *
   * stream_eof() is not currently being used
   *
   * @see StreamWrapperInterface
   *
   * @return bool
   *   TRUE if end-of-file has been reached.
   */

  public function stream_eof() {

    if (variable_get('rackspace_debug')) {
      // Add testing variables here for debugging purposes
      $this->writeWatchdog('Func: ' . __FUNCTION__ . ' Line: ' .__LINE__);
    }
    return TRUE;
  }

  /**
   * Support for fseek()
   *
   * stream_seek() is not currently being used
   *
   * @see StreamWrapperInterface
   *
   * @param int $offset
   *   The byte offset to got to.
   * @param $whence
   *   SEEK_SET, SEEK_CUR, or SEEK_END.
   *
   * @return bool
   *   TRUE on success.
   *
   */
  public function stream_seek($offset, $whence) {

    if (variable_get('rackspace_debug')) {
      // Add testing variables here for debugging purposes
      $this->writeWatchdog('Func: ' . __FUNCTION__ . ' Line: ' .__LINE__ . ' OFFSET ' . $offset);
    }
    return FALSE;
  }

  /**
   * Support for fflush()
   *
   * stream_flush() is not currently being used
   *
   * @see StreamWrapperInterface
   *
   * @return bool
   *   TRUE if data was successfully stored (or there was no data to store).
   *
   */
  public function stream_flush() {

    if (variable_get('rackspace_debug')) {
      // Add testing variables here for debugging purposes
      $this->writeWatchdog('Func: ' . __FUNCTION__ . ' Line: ' .__LINE__);
    }
    return TRUE;
  }

  /**
   * Support for ftell()
   *
   * stream_tell() is not currently being used
   *
   * @see StreamWrapperInterface
   *
   * @return int
   *   The current offset in bytes from the beginning of file.
   *
   */
  public function stream_tell() {

    if (variable_get('rackspace_debug')) {
      // Add testing variables here for debugging purposes
      $this->writeWatchdog('Func: ' . __FUNCTION__ . ' Line: ' .__LINE__);
    }
    return $this->stream_pointer;
  }

  /**
   * Support for fstat()
   *
   * @see StreamWrapperInterface
   *
   * @return array
   *   An array with file status, or FALSE in case of an error - see fstat()
   *   for a description of this array.
   *
   * Adopted from the remote_stream_wrapper contrib module
   */
  public function stream_stat() {

    if (variable_get('rackspace_debug')) {
      // Add testing variables here for debugging purposes
      $this->writeWatchdog('Func: ' . __FUNCTION__ . ' Line: ' .__LINE__);
    }
    $stat = array();
    // Make sure the request comes from an external source
    $path = $this->getExternalUrl();
    // Make sure the request does not fail because of spaces in the filename
    $path = str_replace(' ', '%20', $path);
    $request = drupal_http_request($path, array('method' => 'HEAD'));

    if (empty($request->error)) {
      if (isset($request->headers['content-length'])) {
        $stat['size'] = $request->headers['content-length'];

      } else if ($size = strlen($this->getStreamContent())) {
        // If the HEAD request does not return a Content-Length header, fall
        // back to performing a full request of the file to determine its file
        // size.
        $stat['size'] = $size;
      }
    }
    // Setup the defaults for the stats array
    if (!empty($stat)) {
      $defaults = array(
        'dev' => 0,      // device number
        'ino' => 0,      // inode number
        'mode' => 0100000 | 0444,     // inode protectio
        'nlink' => 0,    // number of links
        'uid' => 0,      // userid of owner
        'gid' => 0,      // groupid of owner
        'rdev' => -1,    // device type, if inode device *
        'size' => 0,     // size in bytes
        'atime' => 0,    // time of last access (Unix timestamp)
        'mtime' => 0,    // time of last modification (Unix timestamp)
        'ctime' => 0,    // time of last inode change (Unix timestamp)
        'blksize' => -1, // blocksize of filesystem IO
        'blocks' => -1,  // number of blocks allocated
      );

      $return = array();
      // Loop through the defaults and match up anything returned from the request
      foreach (array_keys($defaults) as $index => $key) {
        if (!isset($stat[$key])) {
          $return[$index] = $defaults[$key];
          $return[$key] = $defaults[$key];
        }else {
          $return[$index] = $stat[$key];
          $return[$key] = $stat[$key];
        }
      }
      return $return;
    } else {
      return FALSE;
    }
  }

  /**
   * Support for fclose()
   *
   * stream_close() is not currently being used
   *
   * @see StreamWrapperInterface
   *
   * @return bool
   *   TRUE if stream was successfully closed.
   */
  public function stream_close() {

    if (variable_get('rackspace_debug')) {
      // Add testing variables here for debugging purposes
      $this->writeWatchdog('Func: ' . __FUNCTION__ . ' Line: ' .__LINE__);
    }
    return FALSE;
  }

  /**
   * Support for unlink()
   *
   * @see StreamWrapperInterface
   *
   * @param string $uri
   *   A string containing the uri to the resource to delete.
   *
   * @return bool
   *   TRUE if resource was successfully deleted.
   *
   */
  public function unlink($uri) {

    if (variable_get('rackspace_debug')) {
      // Add testing variables here for debugging purposes
      $this->writeWatchdog('Func: ' . __FUNCTION__ . ' Line: ' .__LINE__ . ' URI ' . $uri);
    }
    $this->uri = $uri;

    $target = $this->getLocalPath($this->uri);

    if (!isset($this->container)) {
      try {
        $this->setContainer();
      } catch(Exception $e) {
        $this->writeWatchdog('Func: ' . __FUNCTION__ . ' Line: ' .__LINE__ . ' URI ' . $e->getMessage());
      }
    }

    $deleted = FALSE;
    $doesnotexist = FALSE;
    try {
      // Dont put name here becuase it will fetch the object.
      $obj = $this->container->dataObject();
      // Delete by name, without fetching the object.
      $obj->setName($target);
      $deleted = $obj->delete();
    } catch (NoSuchObjectException $e) {
      $doesnotexist = TRUE;
      $this->writeWatchdog('Func: ' . __FUNCTION__ . ' Line: ' .__LINE__ . ' URI ' . $e->getMessage());
    }

    if ($deleted || $doesnotexist) {
      // Remove file from database.
      $this->remove_file($this->uri);
      return TRUE;
    }
    return FALSE;
  }

  /**
   * Support for rename()
   *
   * rename() is not currently being used
   *
   * @see StreamWrapperInterface
   *
   * @param string $from_uri,
   *   The uri to the file to rename.
   * @param string $to_uri
   *   The new uri for file.
   *
   * @return bool
   *   TRUE if file was successfully renamed.
   */
  public function rename($from_uri, $to_uri) {

    if (variable_get('rackspace_debug')) {
      // Add testing variables here for debugging purposes
      $this->writeWatchdog('Func: ' . __FUNCTION__ . ' Line: ' .__LINE__ . ' FROM URI ' . $from_uri);
    }
    return FALSE;
  }

  /**
   * Support for stat().
   *
   * @param string $uri
   *   A string containing the URI to get information about.
   * @param string $flags
   *   A bit mask of STREAM_URL_STAT_LINK and STREAM_URL_STAT_QUIET.
   *
   * @return array
   *   An array with file status, or FALSE in case of an error - see fstat()
   *   for a description of this array.
   */
  public function url_stat($uri, $flags) {

    if (variable_get('rackspace_debug')) {
      // Add testing variables here for debugging purposes
      $this->writeWatchdog('Func: ' . __FUNCTION__ . ' Line: ' .__LINE__ . ' URI ' . $uri);
    }

    if (!isset($uri)) {
      $uri = $this->uri;
    }
    $this->uri = $uri;

    $target  = $this->getTarget($this->uri);

    // Default to fail.
    $stat = FALSE;
    $mode = 0;

    if (empty($target)) {
      $fileinfo = array('filemime' => 'application/directory', 'filesize' => 1);
    }
    else {
      $fileinfo = $this->get_file_info($this->uri);
    }

    // If file does not exist in the database (its probably new).
    // Try to get it from Rackspace.
    if (!$fileinfo) {
      if (!isset($this->object)) {
        if (!isset($this->container)) {
          try {
            $this->setContainer();
          } catch (Exception $e) {
            $this->writeWatchdog('Func: ' . __FUNCTION__ . ' Line: ' .__LINE__ . ' URI ' . $e->getMessage());
          }
        }
      }

      // Check if the file exists.
      $objexists = TRUE;
      try{
        $this->object = $this->container->dataObject($target);
        $filemime = $this->object->getContentType();
        $filesize = $this->object->getContentLength();

        // File does exist!  add file info to the database.
        $this->add_file($this->uri, $filemime, $filesize);
      } catch (Exception $e) {
        $objexists = FALSE;
        $this->writeWatchdog('Func: ' . __FUNCTION__ . ' Line: ' .__LINE__ . ' URI ' . $e->getMessage());
      }
    } else {
      // Fileinfo exists in the database.
      $objexists = TRUE;
      $filemime = $fileinfo['filemime'];
      $filesize = $fileinfo['filesize'];
    }

    if ($objexists) {
      // If the target is empty, its the root of the container (directory).
      if ($filemime == "application/directory") {
        // S_IFDIR means it's a directory.
        $mode = 0040000;
      } else {
        // S_IFREG, means it's a file.
        $mode = 0100000;
      }

      if ($mode) {
        $mode |= 0777;

        $stat = array(
          'dev' => 0,
          '0' => 0,
          'ino' => 0,
          '1' => 0,
          'mode' => $mode,
          '2' => $mode,
          'nlink' => 0,
          '3' => 0,
          'uid' => 0,
          '4' => 0,
          'gid' => 0,
          '5' => 0,
          'rdev' => 0,
          '6' => 0,
          'size' => $filesize,
          '7' => $filesize,
          'atime' => 0,
          '8' => 0,
          'mtime' => 0,
          '9' => 0,
          'ctime' => 0,
          '10' => 0,
          'blksize' => 0,
          '11' => 0,
          'blocks' => 0,
          '12' => 0,
        );
      }
    }
    return $stat;
  }

  /**
   * Gets the name of the directory from a given path.
   *
   * @param string $uri
   *   A URI.
   *
   * @return string
   *   A string containing the directory name.
   *
   * @see drupal_dirname()
   */

  public function dirname($uri = NULL) {

    if (variable_get('rackspace_debug')) {
      // Add testing variables here for debugging purposes
      $this->writeWatchdog('Func: ' . __FUNCTION__ . ' Line: ' .__LINE__ . ' URI ' . $uri);
    }
    list($scheme, $target) = explode('://', $uri, 2);
    $target  = $this->getTarget($uri);
    $dirname = dirname($target);

    if ($dirname == '.') {
      $dirname = '';
    }
    return $scheme . '://' . $dirname;
  }

  /**
   * Support for mkdir()
   *
   * mkdir() is not currently being used
   *
   * @see StreamWrapperInterface
   *
   * @param string $uri
   *   A string containing the URI to the directory to create.
   * @param $mode
   *   Permission flags - see mkdir().
   * @param $options
   *   A bit mask of STREAM_REPORT_ERRORS and STREAM_MKDIR_RECURSIVE.
   *
   * @return bool
   *   TRUE if directory was successfully created.
   *
   */
  public function mkdir($uri, $mode, $options) {

    if (variable_get('rackspace_debug')) {
      // Add testing variables here for debugging purposes
      $this->writeWatchdog('Func: ' . __FUNCTION__ . ' Line: ' .__LINE__ . ' URI ' . $uri);
    }
    return FALSE;
  }

  /**
   * Support for rmdir()
   *
   * rmdir() is not currently being used
   *
   * @see StreamWrapperInterface
   *
   * @param string $uri
   *   A string containing the URI to the directory to delete.
   * @param $options
   *   A bit mask of STREAM_REPORT_ERRORS.
   *
   * @return bool
   *   TRUE if directory was successfully removed.
   *
   */
  public function rmdir($uri, $options) {

    if (variable_get('rackspace_debug')) {
      // Add testing variables here for debugging purposes
      $this->writeWatchdog('Func: ' . __FUNCTION__ . ' Line: ' .__LINE__ . ' URI ' . $uri);
    }
    $this->uri = $uri;
    return FALSE;
  }

  /**
   * Support for opendir().
   *
   * dir_opendir() is not currently being used
   *
   * @see StreamWrapperInterface
   *
   * @param string $uri
   *   A string containing the URI to the directory to open.
   * @param $options
   *   Unknown (parameter is not documented in PHP Manual).
   *
   * @return bool
   *   TRUE on success.
   *
   */
  public function dir_opendir($uri, $options) {

    if (variable_get('rackspace_debug')) {
      // Add testing variables here for debugging purposes
      $this->writeWatchdog('Func: ' . __FUNCTION__ . ' Line: ' .__LINE__ . ' URI ' . $uri);
    }
    return FALSE;
  }

  /**
   * Support for readdir()
   *
   * dir_readdir() is not currently being used
   *
   * @see StreamWrapperInterface
   *
   * @return mixed
   *   The next filename, or FALSE if there are no more files in the directory.
   */
  public function dir_readdir() {

    if (variable_get('rackspace_debug')) {
      // Add testing variables here for debugging purposes
      $this->writeWatchdog('Func: ' . __FUNCTION__ . ' Line: ' .__LINE__);
    }
    return FALSE;
  }

  /**
   * Support for rewinddir().
   *
   * dir_rewindir() is not currently being used
   *
   * @see StreamWrapperInterface
   *
   * @return bool
   *   TRUE on success.
   */
  public function dir_rewinddir() {

    if (variable_get('rackspace_debug')) {
      // Add testing variables here for debugging purposes
      $this->writeWatchdog('Func: ' . __FUNCTION__ . ' Line: ' .__LINE__);
    }
    $this->directory_pointer = 0;
  }

  /**
   * Support for closedir()
   *
   * dir_closedir() is not currently being used
   *
   * @see StreamWrapperInterface
   *
   * @return bool
   *   TRUE on success.
   *
   */
  public function dir_closedir() {

    if (variable_get('rackspace_debug')) {
      // Add testing variables here for debugging purposes
      $this->writeWatchdog('Func: ' . __FUNCTION__ . ' Line: ' .__LINE__);
    }
    $this->directory_pointer = 0;
    unset($this->directory_list);
    return TRUE;
  }


  /**
   * Implements abstract public function getDirectoryPath().
   *
   * @return string
   *   Returns full URL to a Rackspace Cloud Files domain.
   */
  public function getDirectoryPath() {

    if (variable_get('rackspace_debug')) {
      // Add testing variables here for debugging purposes
      $this->writeWatchdog('Func: ' . __FUNCTION__ . ' Line: ' .__LINE__);
    }
    return "http://" . variable_get('rackspace_domain');
  }

  /**
   * Fetch the content of the file using drupal_http_request().
   * Adopted from remote_stream_wrapper contrib module
   *
   * @return string
   *  data returned from the request
   */
  protected function getStreamContent() {

    if (variable_get('rackspace_debug')) {
      // Add testing variables here for debugging purposes
      $this->writeWatchdog('Func: ' . __FUNCTION__ . ' Line: ' .__LINE__);
    }
    if (!isset($this->$stream_data)) {
      $this->$stream_data = NULL;
      // Make sure the request comes from the external source
      $path = $this->getExternalUrl();
      // Make sure the request does not fail because of spaces in the filename
      $path = str_replace(' ', '%20', $path);
      $request = drupal_http_request($path);
      if (empty($request->error) && !empty($request->data)) {
        $this->$stream_data = $request->data;
      }
    }
    return $this->$stream_data;
  }

  /**
   * Utility function for implementing file_save()
   *
   * @param string $file
   *   A file object
   *
   * @return object
   *  The saved file object..
   */
  public function add_file(stdClass $file) {

    if (variable_get('rackspace_debug')) {
      // Add testing variables here for debugging purposes
      $this->writeWatchdog('Func: ' . __FUNCTION__ . ' Line: ' .__LINE__);
    }
    module_invoke_all('file_presave', $file);
    module_invoke_all('entity_presave', $file, 'file');
    drupal_write_record('file_managed', $file);
    // Inform modules about the newly added file.
    module_invoke_all('file_insert', $file);
    module_invoke_all('entity_insert', $file, 'file');
    // Clear the static loading cache.
    entity_get_controller('file')->resetCache(array($file->fid));
    return $file;
  }

  /**
   * Utility function for updating an existing file
   *
   * @param string $file
   *   A file object
   *
   * @return object
   *  The updated file object..
   */
  public function update_file(stdClass $file) {

    if (variable_get('rackspace_debug')) {
      // Add testing variables here for debugging purposes
      $this->writeWatchdog('Func: ' . __FUNCTION__ . ' Line: ' .__LINE__);
    }
    module_invoke_all('file_presave', $file);
    module_invoke_all('entity_presave', $file, 'file');

    drupal_write_record('file_managed', $file, 'fid');
    // Inform modules that the file has been updated.
    module_invoke_all('file_update', $file);
    module_invoke_all('entity_update', $file, 'file');

    // Clear the static loading cache.
    entity_get_controller('file')->resetCache(array($file->fid));
    return $file;
  }
  /**
   * Utility function for getting file information.
   *
   * @param string $uri
   *   A string containing the URI of the file.
   *
   * @return array
   *   An array containing the file info (uri, filemime, and filesize).
   */
  public function get_file_info($uri) {

    if (variable_get('rackspace_debug')) {
      // Add testing variables here for debugging purposes
      $this->writeWatchdog('Func: ' . __FUNCTION__ . ' Line: ' .__LINE__);
    }
    $query = db_select('file_managed', 'f');
    $query->condition('uri', $uri);
    $query->fields('f', array('uri', 'filemime', 'filesize'));
    $result = $query->execute();
    return $result->fetchAssoc();
  }

  /**
   * Utility function for deleting file information.
   *
   * @param string $uri
   *   A string containing the URI of the file to delete.
   */
  public function remove_file($uri) {

    if (variable_get('rackspace_debug')) {
      // Add testing variables here for debugging purposes
      $this->writeWatchdog('Func: ' . __FUNCTION__ . ' Line: ' .__LINE__);
    }
    return db_delete('file_managed')->condition('uri', $uri)->execute();
  }

  /**
   * Utility function for setting a Rackspace Cloud container.
   *
   */
  public function setContainer() {

    if (variable_get('rackspace_debug')) {
      // Add testing variables here for debugging purposes
      $this->writeWatchdog('Func: ' . __FUNCTION__ . ' Line: ' .__LINE__);
    }
    $info = libraries_load('php-opencloud');

    if ($info['loaded']) {
      $client = $this->getClient();
      $object_store_service = $client->objectStoreService(
        'cloudFiles',
        variable_get('rackspace_region'),
        'publicURL'
      );

      //set the container
      $this->container = $object_store_service->getContainer(
        variable_get('rackspace_container')
      );
    } else {
      $this->container = NULL;
    }
  }

  /**
   * Utility function for getting a Rackspace Cloud Client instance.
   *
   * @return Rackspace
   *   Rackspace client instance.
   */
  public function getClient() {

    if (variable_get('rackspace_debug')) {
      // Add testing variables here for debugging purposes
      $this->writeWatchdog('Func: ' . __FUNCTION__ . ' Line: ' .__LINE__);
    }
    $client = new OpenCloud\Rackspace(variable_get('rackspace_auth_url'), array(
      'username' => variable_get('rackspace_username'),
      'apiKey' => variable_get('rackspace_api_key'),
    ));

    $cache_file = 'private://.nkW54jCnViwzJeHc0OyG';

    // If the cache file exists, try importing it into the client
    if (file_exists($cache_file)) {
      $data = unserialize(file_get_contents($cache_file));
      $client->importCredentials($data);
    }

    $token = $client->getTokenObject();

    // If no token exists, or the current token is expired, re-authenticate and save the new token to disk
    if (!$token || ($token && $token->hasExpired())) {
      $client->authenticate();
      file_put_contents($cache_file, serialize($client->exportCredentials()));
    }
    return $client;
  }

  /**
   * Utility function to write a watchdog record
   *
   * @param $message
   *  The message we are writing to watchdog
   */
  public function writeWatchdog($message) {
    watchdog(
      "cors_file",
      $message,
      array(),
      WATCHDOG_DEBUG
    );
  }
}