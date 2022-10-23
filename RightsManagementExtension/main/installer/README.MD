# Teamcenter Rights Management eXtension 5.0 (Teamcenter RMX) Installation Instruction
- GUI Installation: Right click `install.bat` and select "Run as Administrator" to start installation with GUI mode.
- Silence Installation: Configure different installation parameters in rmx_properties_ui.json file. Then right click `silence\install_silence.bat` and select "Run as Administrator" to start installation.

# Description about the rmx_properties_ui.json file 
This file, either generated after each GUI installation, to use for uninstallation process or pre-configured to run with Silence installation method. The fields inside the json are following:

### temp_dir
- Default and Fixed: Windows - `C:/PROGRA~1/Nextlabs/tmp` or Linux - `/usr/Nextlabs/tmp`
- RMX temp folder, created by installer, used to store some configuration files generated while installation.

### install_dir
- Installation Destination Folder, the folder will be created if it doesn't exist when running RMX installer.
- Example: Windows - `C:/Program Files/Nextlabs` or Linux - `/usr/Nextlabs`

### tc_root
- Teamcenter Root, where Teamcenter is installed on user's machine. 
- Example: `C:/Program Files/Siemens/Teamcenter10`

### tc_data
- Teamcenter Data folder.
- Example: `C:/Program Files/Siemens/tcdata`

### disp_root
- Teamcenter Dispatcher Root folder, the parent directory of AdminClient, DispatcherClient and Module
- Example: `C:/SPLM/DispRoot`

### tc_user
- Teamcenter user. Can be a normal user.

### solution_type
- **Protection in Teamcenter Volume**

### installation_mode
- Valid values: **install** , **remove** , **upgrade**

### infodba_psd
- Password of Teamcenter Root Administrator infodba

### SkyDRM configuration parameters
- skydrm_router_url
- skydrm_app_id
- skydrm_app_key

### jpc_eva_port
- 1099 (Fixed)

### jpc_remote_ip
- Java Policy Controller IP Address 

### tcsoa_hostname
- Teamcenter SOA Hostname
- Example : `https://tcServer01:8080/tc`

### feature_list (after a GUI Installation)
- Contains list of features installed

### feature_status (if used for silence installation)
- Will be set automatically after a GUI Installation. Need configure manually if run with silence installer.
- Valid values: `ON` , `OFF`
- The values are used to specify RMX features to be installed.
- There are some requirements for our feature to be install
- The corresponding feature full names are:

| Field                         | Teamcenter Feature Install Requirements                       |
| ----------------------------- | ------------------------------------------------------------- |
| nxl_template                  | Translation Service Database Module AND Teamcenter Foundation |
| foundation_customization      | Teamcenter Foundation                                         |
| batch_protection              | Teamcenter Foundation                                         |
| workflow_initial_protection   | Teamcenter Foundation OR Teamcenter Rich Client 2-tier        |
| active_workspace_support      | Active Workspace Client (Java EE)                             | 
| nxl_ipem_template             | Integration for Creo                                          |
| dispatcher_module             | Dispatcher Server                                             |
| dispatcher_client_extension   | Dispatcher Client                                             |
| translator_rights_checker     | Dispatcher Server                                             | 
| user_initial_protection       | Teamcenter Rich Client                                        | 
| proxy_runner                  | Teamcenter Rich Client                                        |
| partialdownload               | Teamcenter Rich Client                                        |
| nx_integration                | Siemens NX Installed                                          |
| creo_integration              | PTC Creo Parametric Installed                                 |