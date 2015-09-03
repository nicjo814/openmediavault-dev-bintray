<?php
/**
 * Copyright (c) 2015 OpenMediaVault Plugin Developers
 *
 * @category OMVRpcServiceBintray
 * @package  Openmediavault-dev-bintray
 * @author   OpenMediaVault Plugin Developers <plugins@omv-extras.org>
 * @license  http://www.gnu.org/copyleft/gpl.html GNU General Public License
 * @link     https://github.com/OpenMediaVault-Plugin-Developers/openmediavault-dev-bintray
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

require_once "openmediavault/object.inc";
require_once "openmediavault/config.inc";
require_once "openmediavault/error.inc";
require_once "openmediavault/util.inc";
require_once "openmediavault/rpcservice.inc";
require_once "openmediavault/notify.inc";

/**
 * RPC class for the Bintray plugin
 *
 * @category Class
 * @package  Openmediavault-dev-bintray
 * @author   OpenMediaVault Plugin Developers <plugins@omv-extras.org>
 * @license  http://www.gnu.org/copyleft/gpl.html GNU General Public License
 * @link     https://github.com/OpenMediaVault-Plugin-Developers/openmediavault-dev-bintray
 *
 */
class OMVRpcServiceBintray extends OMVRpcServiceAbstract
{
    /**
     * Get the name of the RPC service
     *
     * @return string
     */
    public function getName()
    {
        return "Bintray";  // RPC Service name. Same as in .js files
    }

    /**
     * Get the main event message path of the service. This is a helper
     * function to avoid "magic numbers".
     *
     * @return string
     */
    private function _getEventMessagePath()
    {
        return "org.openmediavault.services.bintray";
    }

    /**
     * Get the base XPath of the service. This is a helper function to avoid
     * "magic numbers".
     *
     * @return string
     */
    private function _getXpath()
    {
        return "/config/services/bintray";
    }
    
    /**
     * Do a curl API call with given url
     *
     * @param string $url The URL to use in the call
     *
     * @return string
     */
    private function _doApiCall($url)
    {
        $curl = curl_init();
        curl_setopt_array(
            $curl, array(
                CURLOPT_RETURNTRANSFER => 1,
                CURLOPT_TIMEOUT => 30,
                CURLOPT_CONNECTTIMEOUT => 5
            )
        );
        curl_setopt($curl, CURLOPT_URL, $url);
        if (!($response = curl_exec($curl))) {
            throw new OMVException(
                OMVErrorMsg::E_EXEC_MISC,
                'Error: "' . curl_error($curl) . '" - Code: ' . curl_errno($curl)
            );
        }
        curl_close($curl);
        return $response;
    }

    /**
     * Initialize the RPC service. Different methods of the RPC service are
     * declared here
     *
     * @return void
     */
    public function initialize()
    {
        $this->registerMethod("getRepoList");
        $this->registerMethod("getPackageList");
        $this->registerMethod("getFileList");
        $this->registerMethod("getPackages");
        $this->registerMethod("getPackage");
        $this->registerMethod("setPackage");
        $this->registerMethod("deletePackage");
        $this->registerMethod("publishFile");

        $this->registerMethod("getSettings");
        $this->registerMethod("setSettings");
    }

    /**
     * Retrieve all Bintray repos that the user has access to
     * 
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return array $object An ssociative array with all settings
     */
    public function getRepoList($params, $context)
    {
        global $xmlConfig;
        $this->validateMethodContext(
            $context,
            array("role" => OMV_ROLE_ADMINISTRATOR)
        );

        $settings = $xmlConfig->get($this->_getXpath());
        if (is_null($settings)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->_getXpath()
            );
        }

        $objects = array();
        $url = "https://api.bintray.com/repos/" . $settings['username'];
        $data = json_decode($this->_doApiCall($url));
        foreach ($data as $repo) {
            if (preg_match('/^' . $params['omvversion'] . '.*$/', $repo->name)) {
                array_push(
                    $objects,
                    array("repository" => $repo->name)
                );
            }
        }
        return $objects;
    }

    /**
     * Retrieve all Bintray packages in a specific repo
     * 
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return array $object An ssociative array with all settings
     */
    public function getPackageList($params, $context)
    {
        global $xmlConfig;
        $this->validateMethodContext(
            $context,
            array("role" => OMV_ROLE_ADMINISTRATOR)
        );

        $settings = $xmlConfig->get($this->_getXpath());
        if (is_null($settings)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->_getXpath()
            );
        }

        $objects = array();
        $url = "https://api.bintray.com/repos/" .
            "openmediavault-plugin-developers/" .
            $params['repository'] . "/packages";
        $data = json_decode($this->_doApiCall($url));
        foreach ($data as $package) {
            array_push(
                $objects,
                array("bpackage" => $package->name)
            );
        }
        return $objects;
    }

    /**
     * Retrieve all files in location specified by package
     * 
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return array $object An ssociative array with all settings
     */
    public function getFileList($params, $context)
    {
        global $xmlConfig;
        $this->validateMethodContext(
            $context,
            array("role" => OMV_ROLE_ADMINISTRATOR)
        );

        $objects = array();
        $package = $xmlConfig->get($this->_getXpath() . "/packages/package[uuid='" . $params['uuid'] . "']");
        if (is_null($package)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->_getXpath()
            );
        }

        $files = scandir($package['srcpath']);
        foreach ($files as $file) {
            if (preg_match('/^' . $package['bpackage'] . '.*.deb$/', $file)) {
                if (!(is_dir($package['srcpath'] . "/" . $file))) {
                    array_push(
                        $objects,
                        array(
                            "filename" => $file,
                            "fullpath" => $package['srcpath'] . "/" . $file
                        )
                    );
                }
            }
        }

        return $objects;
    }


    /**
     * Retrieve all Bintray packages
     * 
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return array $object An ssociative array with all settings
     */
    public function getPackages($params, $context)
    {
        global $xmlConfig;
        $this->validateMethodContext(
            $context,
            array("role" => OMV_ROLE_ADMINISTRATOR)
        );

        $objects = array();
        $packages = $xmlConfig->getList($this->_getXpath() . "/packages/package");
        if (is_null($packages)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->_getXpath()
            );
        }

        return $packages;
    }

    /**
     * Retrieve a package object
     * 
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return array $object An ssociative array with all settings
     */
    public function getPackage($params, $context)
    {
        global $xmlConfig;
        $this->validateMethodContext(
            $context,
            array("role" => OMV_ROLE_ADMINISTRATOR)
        );

        $objects = array();
        $package = $xmlConfig->get($this->_getXpath() . "/packages/package[uuid='" . $params['uuid'] . "']");
        if (is_null($package)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->_getXpath()
            );
        }

        return $package;
    }

    /**
     * Set a package object
     * 
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return array $object An associative array with all the new settings
     */
    public function setPackage($params, $context)
    {
        global $xmlConfig;
        $this->validateMethodContext(
            $context,
            array("role" => OMV_ROLE_ADMINISTRATOR)
        );
        $this->validateMethodParams(
            $params,
            '{' .
            '"type": "object",' .
            '"properties": {' .
            '"uuid": { "type": "string" },' .
            '"omvversion": { "type": "string" },' .
            '"repository": { "type": "string" },' .
            '"bpackage": { "type": "string" },' .
            '"srcpath": { "type": "string" },' .
            '"dist": { "type": "string" },' .
            '"arch": { "type": "string" }' .
            '}}'
        );

        $object = array(
            "uuid" => (strcmp($params['uuid'], "undefined") === 0) ? OMVUtil::uuid() : $params['uuid'],
            "omvversion" => $params['omvversion'],
            "repository" => $params['repository'],
            "bpackage" => $params['bpackage'],
            "srcpath" => $params['srcpath'],
            "dist" => $params['dist'],
            "arch" => $params['arch']
        );

        if (strcmp($params['uuid'], "undefined") === 0) {
            //New package
            if (false === $xmlConfig->set($this->_getXpath() . "/packages", array("package" => $object))) {
                throw new OMVException(
                    OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED,
                    $this->_getXpath()
                );
            }
        } else {
            //Update existing package
            $xpath = $this->_getXpath() . "/packages/package[uuid='" . $object['uuid'] . "']";
            $oldObject = $xmlConfig->get($xpath);
            if (is_null($oldObject)) {
                throw new OMVException(
                    OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                    $xpath
                );
            }
            if (false === $xmlConfig->replace($xpath, $object)) {
                throw new OMVException(
                    OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED,
                    $this->_getXpath()
                );
            }
        }

        return $object;
    }

    /**
     * Delete a package object
     * 
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return array $object An associative array with all the new settings
     */
    public function deletePackage($params, $context)
    {
        global $xmlConfig;
        $this->validateMethodContext(
            $context,
            array("role" => OMV_ROLE_ADMINISTRATOR)
        );
        $this->validateMethodParams(
            $params,
            '{' .
            '"type": "object",' .
            '"properties": {' .
            '"uuid": { "type": "string" }' .
            '}}'
        );

        $xpath = $this->_getXpath() . "/packages/package[uuid='" . $params['uuid'] . "']";
        if (false === $xmlConfig->delete($xpath)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED,
                $this->_getXpath()
            );
        }
    }

    /**
     * Publish a file to Bintray
     * 
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return array $object An associative array with all the new settings
     */
    public function publishFile($params, $context)
    {
        global $xmlConfig;
        $this->validateMethodContext(
            $context,
            array("role" => OMV_ROLE_ADMINISTRATOR)
        );
        $this->validateMethodParams(
            $params,
            '{' .
            '"type": "object",' .
            '"properties": {' .
            '"uuid": { "type": "string" },' .
            '"file": { "type": "string" }' .
            '}}'
        );

        $settings = $xmlConfig->get($this->_getXpath());
        if (is_null($settings)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->_getXpath()
            );
        }

        $package = $xmlConfig->get($this->_getXpath() . "/packages/package[uuid='" . $params['uuid'] . "']");
        if (is_null($package)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->_getXpath()
            );
        }

        if (!(preg_match('/^.*_([\d]+\.[\d\.]+).*$/', $params['file'], $matches))) {
            throw new OMVException(
                OMVErrorMsg::E_EXEC_MISC,
                "Failed to get version from filename:" .
                $params['file']
            );

        }
        $version = $matches[1];

        //Can't get php curl to work with authentication...
        /*
        $url = "https://api.bintray.com/content/openmediavault-plugin-developers/" .
            $package['repository'] . "/" . $package['bpackage'] .
            "/" . $version . "/" . "pool/main/" . substr($package['bpackage'], 0, 1) .
            "/" . $package['bpackage'] . "/" . $params['file'] . ";deb_distribution=" .
            $package['dist'] . ";deb_component=main;deb_architecture=" . $package['arch'] .
            ";publish=1";

        $curl_options = array(
            CURLOPT_RETURNTRANSFER => 1,
            CURLOPT_TIMEOUT => 30,
            CURLOPT_CONNECTTIMEOUT => 5,
            CURLOPT_PUT => true,
            CURLOPT_INFILE => $package['srcpath'] . "/" . $params['file'],
            CURLOPT_INFILESIZE => filesize($package['srcpath'] . "/" .$params['file']),
            CURLOPT_HEADER => true,
            CURLOPT_HTTPHEADER => array(
                'X-GPG-PASSPHRASE: ' . $settings['gpgpass'],
                "Authorization: Basic " . base64_encode($settings['username'] . ":" . $settings['apikey'])
            )
        );


        $curl = curl_init();
        curl_setopt_array($curl, $curl_options);
        curl_setopt($curl, CURLOPT_URL, $url);
        if (!($response = curl_exec($curl))) {
            throw new OMVException(
                OMVErrorMsg::E_EXEC_MISC,
                'Error: "' . curl_error($curl) . '" - Code: ' . curl_errno($curl)
            );
        }
        curl_close($curl);
         */

        $url = "https://api.bintray.com/content/openmediavault-plugin-developers/" .
            $package['repository'] . "/" . $package['bpackage'] .
            "/" . $version . "/" . "pool/main/" . substr($package['bpackage'], 0, 1) .
            "/" . $package['bpackage'] . "/" . $params['file'];

        $cmd = "curl -X PUT -T " . $package['srcpath'] . "/" . $params['file'] . " " .
            "-H 'X-GPG-PASSPHRASE: " . $settings['gpgpass'] . "' " .
            "-H 'X-Bintray-Debian-Distribution: " . $package['dist'] . "' " .
            "-H 'X-Bintray-Debian-Component: main' " .
            "-H 'X-Bintray-Debian-Architecture: " . $package['arch'] . "' " .
            "-H 'X-Bintray-Publish: 1' " .
            "-u" . $settings['username'] . ":" . $settings['apikey'] . " " . $url;

        OMVutil::exec($cmd, $out, $res);

        if (!(strcmp($out[0], "{}") === 0)) {
            throw new OMVException(
                OMVErrorMsg::E_EXEC_MISC,
                $out[0]
            );
        }
    }


    /**
     * Retrieve all plugin settings
     * 
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return array $object An ssociative array with all settings
     */
    public function getSettings($params, $context)
    {
        global $xmlConfig;
        $this->validateMethodContext(
            $context,
            array("role" => OMV_ROLE_ADMINISTRATOR)
        );

        $object = $xmlConfig->get($this->_getXpath());
        if (is_null($object)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->_getXpath()
            );
        }

        return $object;
    }

    /**
     * Set all plugin settings
     * 
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return array $object An associative array with all the new settings
     */
    public function setSettings($params, $context)
    {
        global $xmlConfig;
        $this->validateMethodContext(
            $context,
            array("role" => OMV_ROLE_ADMINISTRATOR)
        );
        $this->validateMethodParams(
            $params,
            '{' .
            '"type": "object",' .
            '"properties": {' .
            '"username": {	"type": "string" },' .
            '"apikey": {	"type": "string" },' .
            '"enabled": {	"type": "boolean" },' .
            '"gpgpass": {	"type": "string" }' .
            '}}'
        );

        $oldSettings = $xmlConfig->get($this->_getXpath());
        if (is_null($oldSettings)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->_getXpath()
            );
        }

        // Update the configuration object.
        $object = array(
            "enabled" => array_boolval($params, "enabled"), 
            "username" => $params['username'],
            "apikey" => $params['apikey'],
            "gpgpass" => $params['gpgpass'],
            "packages" => $oldSettings['packages']
        );

        if (false === $xmlConfig->replace($this->_getXpath(), $object)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED,
                $this->_getXpath()
            );
        }

        $dispatcher = &OMVNotifyDispatcher::getInstance();
        $dispatcher->notify(
            OMV_NOTIFY_MODIFY,
            $this->_getEventMessagePath(),
            $object
        );

        return $object;
    }


    //throw new OMVModuleDockerException($cmd);
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceBintray());
