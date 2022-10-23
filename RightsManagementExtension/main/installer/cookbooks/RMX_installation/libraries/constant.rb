# Teamcenter RMX 5.0
# Cookbook Name :: RMX_installation
# Library       :: Constant Library
# Description   :: Contains constants used by recipes
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

# InstallFile Constant: General File name
module InstallFile
    # Created in tmp dir to determine whether RMX has been installed
    HAS_INSTALLED_TXT = 'has_installed.txt'.freeze
end

# Script Constant: Batch Script Files (.bat, .sh, ...)
module Script
    # Batch script extension: .bat in Window and .sh in Linux
    SCRIPT_EXT = '.bat'.freeze if RUBY_PLATFORM =~ /mswin|mingw|windows/
    SCRIPT_EXT = '.sh'.freeze if RUBY_PLATFORM =~ /linux/

    # Some utility using during installation
    RMX_TRANSLATOR = 'translator' + SCRIPT_EXT
    ENCRYPTCONFIGPASSWORD = 'encryptConfigPassword' + SCRIPT_EXT
    TEMPLATE_MODIFIER = 'templatemod' + SCRIPT_EXT
    BMIDE_PROCESSTEMPLATES = 'bmide_processtemplates' + SCRIPT_EXT
    INSTALL_RUNNER = 'install_runner' + SCRIPT_EXT
    BMIDE_SETUPKNOWLEDGEBASE = 'bmide_setupknowledgebase' + SCRIPT_EXT
    BMIDE_MODELTOOL = 'bmide_modeltool' + SCRIPT_EXT
    BMIDE_GENERATETCPLMXMLSCHEMA = 'bmide_generatetcplmxmlschema' + SCRIPT_EXT
    TCCS_CLASSPATH = 'tccs_classpath' + SCRIPT_EXT
    GENREGXML = 'genregxml' + SCRIPT_EXT
    RUNDISPATCHERCLIENT = 'runDispatcherClient' + SCRIPT_EXT
    RUNMODULE = 'runmodule' + SCRIPT_EXT

    TC_PROFILEVARS = RUBY_PLATFORM =~ /mswin|mingw|windows/ ? 'tc_profilevars' + SCRIPT_EXT : 'tc_profilevars'
end

# RMX Library Constant: Library Files (.dll, .so, ...)
module RMXLibrary
    LIB_EXT = '.dll'.freeze if RUBY_PLATFORM =~ /mswin|mingw|windows/
    LIB_EXT = '.so'.freeze if RUBY_PLATFORM =~ /linux/
    NXLAUTOPROTECT = 'NxlAutoProtect' + LIB_EXT
    LIBNXL3_EXTENSION_LIBRARY = 'libNxl3_extension_library' + LIB_EXT
    NXLUTILS64 = 'NxlUtils64' + LIB_EXT
    LIBNXLUTILS = 'libNxlUtils' + LIB_EXT
end

# TeamcenterProcess Constant: Teamcenter Process Utility (.exe)
module TeamcenterProcess
    EXE_EXT = '.exe'.freeze if RUBY_PLATFORM =~ /mswin|mingw|windows/
    EXE_EXT = ''.freeze if RUBY_PLATFORM =~ /linux/
    GENERATE_METADATA_CACHE = 'generate_metadata_cache' + EXE_EXT
    GENERATE_CLIENT_META_CACHE = 'generate_client_meta_cache' + EXE_EXT
    BMIDE_MANAGE_TEMPLATES = 'bmide_manage_templates' + EXE_EXT
    PREFERENCES_MANAGER = 'preferences_manager' + EXE_EXT
    TEAMCENTER_EXE = 'Teamcenter' + EXE_EXT
    TAO_IMR_LOCATOR = 'tao_imr_locator' + EXE_EXT
    TAO_IMR_ACTIVATOR = 'tao_imr_activator' + EXE_EXT
    XSLT_UTILITY = 'plmxml_tm_edit_xsl' + EXE_EXT
    BMIDE_GENERATE_CLIENT_CACHE = RUBY_PLATFORM =~ /mswin|mingw|windows/ ? 'bmide_generate_client_cache' : 'bmide_generate_client_cache.sh'   # .bat in Windows and no extension in Linux
end

# Installer Constant: Teamcenter Feature ID, RMX Feature list
module Installer
    # Teamcenter constant feature id
    AWS_RUNTIME_SERVER = '24E2685325021872095B1249CDE5B49B'.freeze
    VIS_SERVER_MANAGER = 'VDR04GFQPHUG7SBBS91XYV1E1A41UJ6Y'.freeze

    # Teamcenter EDRM full features list
    RMX_FEATURES = %w(
        nxl_template
        foundation_customization
        batch_protection
        workflow_initial_protection
        active_workspace_support
        scf_integration
        nxl_ipem_template
        nxl_swim_template
        dispatcher_module
        dispatcher_client_extension
        translator_rights_checker
        user_initial_protection
        rac_rmx
        proxy_runner
        partialdownload
        bb_integration
        nx_integration
        creo_integration
        solidworks_integration
        autocad_integration
        catia_integration
        rights_management_desktop
    ).freeze
end

# BB Plugin Constant: Briefcase Browser Plugin Name (use only by BBIntegration)
module BBPlugin
    # Briefcase Browser Integration
    NXPOSTOPEN = 'com.teamcenter.bce.internal.cad.nx.NXPostOpen'.freeze
    NXLNXPOSTOPEN = 'com.teamcenter.bce.internal.cad.nx.NxlNXPostOpen'.freeze
end
