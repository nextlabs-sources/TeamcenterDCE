require 'win32/service' if RUBY_PLATFORM =~ /mswin|mingw|windows/
require 'win32/registry' if RUBY_PLATFORM =~ /mswin|mingw|windows/
require 'win32ole' if RUBY_PLATFORM =~ /mswin|mingw|windows/

# Helper for finding SolidWorks SWIM dir (swim_dir_in_registry)
# Reference: https://stackoverflow.com/questions/50713867/ruby-how-to-read-registry-keys-for-32-bit-and-64-bit-applications-from-32-bit
if RUBY_PLATFORM =~ /mswin|mingw|windows/
    module Win32::Registry::Constants
        KEY_WOW64_64KEY = 0x0100
        KEY_WOW64_32KEY = 0x0200
    end
end

module RegistryFinder

    def get_rmd_existing_version()
        registry = Win32::Registry::HKEY_LOCAL_MACHINE
        control_panel = 'SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall'
        rmd_name = 'SkyDRM Desktop for Windows'

        begin
            registry.open(control_panel, Win32::Registry::KEY_READ | Win32::Registry::KEY_WOW64_64KEY) do |program_list|
                program_list.each_key do |program, _wtime|
                    final_key = (control_panel + '\\' + program)
                    registry.open(final_key, Win32::Registry::KEY_READ | Win32::Registry::KEY_WOW64_64KEY) do |final_registry|
                        final_registry.each_value do |name, _type, data|
                            if name == "DisplayName" && data.include?(rmd_name)
                                _ , version = final_registry.read("DisplayVersion")
                                version = version.split('.')[-1]
                                return [true, version]
                            end
                        end
                    end
                end
            end
        rescue
            return [false, '']
        end
        return [false, '']
    end

    # Check if Creo Parametric 4.0/7.0 is installed
    def creo_dir_in_registry
        return unless RUBY_PLATFORM =~ /mswin|mingw|windows/
        registry = Win32::Registry::HKEY_LOCAL_MACHINE

        creo_parametric_key7 = 'SOFTWARE\\PTC\\PTC Creo Parametric'
        creo_parametric_key4 = 'SOFTWARE\\Wow6432Node\\PTC\\PTC Creo Parametric\\4.0'
        creo_install_dir = ''
        creo_ver = ''

        # Find Creo 7.0 as piority
        begin
            # Creo 7.0 registry belong to 64-bit section so the reg type need include KEY_WOW64_64KEY
            reg_type = Win32::Registry::KEY_READ | Win32::Registry::KEY_WOW64_64KEY
            registry.open(creo_parametric_key7, reg_type) do |reg|
                unless reg.keys.empty?
                    # Query version next (Ex: 7.0.1.0)
                    version_subkey = reg.keys[0]
                    final_query_key = creo_parametric_key7 + '\\' + version_subkey
                    registry.open(final_query_key, reg_type) do |final_reg|
                        # Full registry: Contains all key, values for CreoParametric installation if version correct
                        final_reg.each_value do |name, _type, data|
                            creo_install_dir = data if name == 'InstallDir'
                            creo_ver = data if name == 'Release'
                        end
                        if creo_ver.start_with?('7')
                            return creo_install_dir
                        end
                    end
                end
            end
        rescue Win32::Registry::Error, StandardError
            # Can't find Creo Parametric 7.0 registry
            creo_install_dir = ''
        end

        # Find Creo 4.0 if 7.0 not found
        begin
            registry.open(creo_parametric_key4) do |reg|
                unless reg.keys.empty?
                    # Query Datecode next (Ex: 2018070)
                    datecode_subkey = reg.keys[0]
                    final_query_key = creo_parametric_key4 + '\\' + datecode_subkey
                    registry.open(final_query_key) do |final_reg|
                        # Full registry: Contains all key, values for CreoParametric installation
                        final_reg.each_value do |name, _type, data|
                            creo_install_dir = data if name == 'InstallDir'
                            creo_ver = data if name == 'Release'
                        end
                        if creo_ver.start_with?('4')
                            return creo_install_dir
                        else        # Wrong version found?
                            return ''
                        end
                    end
                end
            end
        rescue Win32::Registry::Error, StandardError
            # Can't find Creo Parametric 4.0 registry. Not found Creo InstallDir
            creo_install_dir = ''
        end
        
        creo_install_dir
    end

    # Check if and where Creo Integration module (IPEM) is installed
    def ipem_dir_in_registry
        return unless RUBY_PLATFORM =~ /mswin|mingw|windows/
        registry = Win32::Registry::HKEY_LOCAL_MACHINE
        control_panel = 'SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall'
        tc_ipem_name = 'Teamcenter Integration for Creo';
        ipem_dir = ''

        begin
            registry.open(control_panel, Win32::Registry::KEY_READ | Win32::Registry::KEY_WOW64_64KEY) do |program_list|
                program_list.each_key do |program, _wtime| 
                    if program.include?(tc_ipem_name)
                        ipem = control_panel + '\\' + program
                        registry.open(ipem, Win32::Registry::KEY_READ | Win32::Registry::KEY_WOW64_64KEY) do |ipem_registry|
                            ipem_registry.each_value do |name, _type, data|
                                ipem_dir = data if name.start_with?('InstallLocation')
                            end
                        end		
                    end
                end
            end
        rescue
            # Can't find Teamcenter Integration for SolidWorks in registry
            ipem_dir = ''
        end
        ipem_dir
    end

    # Check if and where SolidWorks is installed
    def solidworks_dir_in_registry
        return unless RUBY_PLATFORM =~ /mswin|mingw|windows/
        registry = Win32::Registry::HKEY_LOCAL_MACHINE
        solidworks_dir_key = 'SOFTWARE\Wow6432Node\SolidWorks\IM'
        solidworks_install_dir = ''
        begin
            registry.open(solidworks_dir_key) do |reg|
                reg.each_value do |name, _type, data|
                    solidworks_install_dir = data if name.start_with?('InstallDir')
                end
                return solidworks_install_dir
            end
        rescue Win32::Registry::Error, StandardError
            # Can't find Creo Parametric 4.0 registry
            solidworks_install_dir = ''
        end
        solidworks_install_dir
    end

    # Check if and where SolidWorks Integration module (SWIM) is installed
    def swim_dir_in_registry
        return unless RUBY_PLATFORM =~ /mswin|mingw|windows/
        registry = Win32::Registry::HKEY_LOCAL_MACHINE
        control_panel = 'SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall'
        tc_swim_name = 'Teamcenter Integration for SolidWorks';
        swim_dir = ''
        begin
            registry.open(control_panel, Win32::Registry::KEY_READ | Win32::Registry::KEY_WOW64_64KEY) do |program_list|
                program_list.each_key do |program, _wtime| 
                    if program.include?(tc_swim_name)
                        swim = control_panel + '\\' + program
                        registry.open(swim, Win32::Registry::KEY_READ | Win32::Registry::KEY_WOW64_64KEY) do |swim_registry|
                            swim_registry.each_value do |name, _type, data|
                                swim_dir = data if name.start_with?('InstallLocation')
                            end
                        end		
                    end
                end
            end
        rescue
            # Can't find Teamcenter Integration for SolidWorks in registry
            swim_dir = ''
        end
        swim_dir
    end

    # Check if and where AutoCAD is installed
    def autocad_dir_in_registry
        return unless RUBY_PLATFORM =~ /mswin|mingw|windows/
        registry = Win32::Registry::HKEY_LOCAL_MACHINE
        control_panel = 'SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall'
        autocad_software_name = "AutoCAD 2022"
        autocad_registry_code = "{28B89EEF-5101-0000-0102-CF3F3A09B77D}"
        autocad_dir = ''
        begin
            registry.open(control_panel, Win32::Registry::KEY_READ | Win32::Registry::KEY_WOW64_64KEY) do |program_list|
                program_list.each_key do |program, _wtime| 
                    if program.include?(autocad_registry_code)
                        final_entry = control_panel + '\\' + autocad_registry_code
                        registry.open(final_entry, Win32::Registry::KEY_READ | Win32::Registry::KEY_WOW64_64KEY) do |acad_registry|
                            _ , display_name = acad_registry.read("DisplayName")
                            if display_name.start_with?(autocad_software_name)
                                _ , autocad_dir = acad_registry.read("InstallLocation")
                                return autocad_dir
                            end
                        end		
                    end
                end
            end
        rescue Win32::Registry::Error, StandardError
            # Can't find AutoCAD 2022 software
            autocad_dir = ''
        end
        autocad_dir
    end

    # Check if and where CATIA is installed
    def catia_dir_in_registry
        return unless RUBY_PLATFORM =~ /mswin|mingw|windows/

        catia_dir = ''
        registry = Win32::Registry::HKEY_LOCAL_MACHINE
        catia_app_registry = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\CNEXT.exe"

        begin
            registry.open(catia_app_registry, Win32::Registry::KEY_READ | Win32::Registry::KEY_WOW64_64KEY) do |app_registry|
                _ , app_exe = app_registry.read("")
                catia_dir = File.expand_path('../../..', app_exe)
            end
        rescue Win32::Registry::Error, StandardError
            catia_dir = ''
        end

        catia_dir
    end
end