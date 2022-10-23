
require_relative './configuration'
require_relative './utility'

require 'fileutils'

class FIPSSDKUpdater
    include Utility
    attr_accessor :fips_configuration, :fips_sdk_folder

    STANDARD_SDK_LIST = %w(
        bcpkix-jdk15on-1.64.jar
        bcprov-jdk15on-1.64.jar
        common-framework.jar
        commons-cli-1.2.jar
        commons-codec-1.10.jar
        commons-lang3-3.5.jar
        crypt.jar
        gson-2.3.1.jar
        guava-18.0.jar
        log4j-api-2.10.0.jar
        log4j-core-2.10.0.jar
        rmjavasdk-ng.jar
        shared.jar
    )

    def initialize
        @fips_configuration = Configuration.instance
        @fips_sdk_folder = File.join(__dir__, 'sdk')
    end

    def update_sdk
        unless @fips_configuration.feature_list.include?('dispatcher_module') || (@fips_configuration.feature_list.include?('partialdownload') && installed_partialdownload_server_solution?)
            puts('No TcDRM configuration here need FIPS SDK Library Update')
            return
        end

        update_general_sdk

        if @fips_configuration.feature_list.include?('dispatcher_module')
            update_translator_sdk
        end

        if @fips_configuration.feature_list.include?('partialdownload') && installed_partialdownload_server_solution?
            stop_fcc
            update_partialdownload_sdk
            start_fcc
        end
    end

    def installed_partialdownload_server_solution?
        # In Server solution, a bunch of RMJavaSDK jar is copy to third party folder.
        # Check a nextlabs library exist is the way to differentiate server and client solution
        tccs_folder = File.join(@fips_configuration.tc_root, 'tccs')
        nxl_library_folder = File.join(tccs_folder, 'third_party', 'nxl')
        return File.exist?(File.join(nxl_library_folder, 'rmjavasdk-ng.jar'))
    end

    def update_general_sdk
        # In general TcDRM folder (@install_dir), we need to replace RMJavaSDK from Translator, TranslatorRightsChecker and PartialDownload folder
        translator_folder = File.join(@fips_configuration.install_dir, 'TcDRM', 'Translator', 'tonxlfile', 'bin')

        delete_files(translator_folder, STANDARD_SDK_LIST)
        copy_files(@fips_sdk_folder, translator_folder)

        trc_folder = File.join(@fips_configuration.install_dir, 'TcDRM', 'TranslatorRightsChecker')

        delete_files(trc_folder, STANDARD_SDK_LIST)
        copy_files(@fips_sdk_folder, trc_folder)

        partial_download_folder = File.join(@fips_configuration.install_dir, 'TcDRM', 'PartialDownload')

        delete_files(partial_download_folder, STANDARD_SDK_LIST)
        copy_files(@fips_sdk_folder, partial_download_folder)

        edit_translator_script_file
    end

    def edit_translator_script_file
        puts('Updating TcDRM Translator(+RightsChecker) SDK')
        # Edit translator.bat file (Windows)
        bat_file = File.join(@fips_configuration.install_dir, 'TcDRM', 'Translator', 'tonxlfile', 'bin', 'translator.bat')
        new_bat_file = File.join(@fips_configuration.install_dir, 'TcDRM', 'Translator', 'tonxlfile', 'bin', 'translator_fips.bat')

        before_classpath = 'java -Dlog4j.configurationFile="%RMX_HOME%log4j.properties" -cp '
        after_classpath = ' com.nextlabs.drm.rmx.translator.TranslatorApp %PARAM1% %PARAM2% %PARAM3% %PARAM4% %PARAM5% %PARAM6% %PARAM7% %PARAM8% %PARAM9% %1 %2 %3 %4 %5 %6'
        classpath = build_classpath(get_file_basenames(@fips_sdk_folder))

        file = File.new(new_bat_file, 'w+')
        File.foreach(bat_file) do |line|
            if line.start_with?(before_classpath)
                new_java_call = before_classpath + classpath + after_classpath
                file.puts(new_java_call)
            else
                file.puts(line)
            end
        end
        file.close

        # Edit translator.sh file (Linux)
        sh_file = File.join(@fips_configuration.install_dir, 'TcDRM', 'Translator', 'tonxlfile', 'bin', 'translator.sh')
        new_sh_file = File.join(@fips_configuration.install_dir, 'TcDRM', 'Translator', 'tonxlfile', 'bin', 'translator_fips.sh')

        before_classpath_linux = 'java -Dlog4j.configurationFile="${RMX_HOME}log4j.properties" -cp '
        after_classpath_linux = ' com.nextlabs.drm.rmx.translator.TranslatorApp "$@"'
        classpath = build_classpath_linux(get_file_basenames(@fips_sdk_folder))

        file = File.new(new_sh_file, 'w+')
        File.foreach(sh_file) do |line|
            if line.start_with?(before_classpath_linux)
                new_java_call = before_classpath_linux + classpath + after_classpath_linux
                file.puts(new_java_call)
            else
                file.puts(line)
            end
        end
        file.close
    end

    def build_classpath(filelist)
        # Some addition library need to manually add (translator.jar, slf4j-api-1.7.21.jar and slf4j-log4j12-1.7.21.jar)
        classpath = '"%RMX_HOME%.";"%RMX_HOME%translator.jar"'
        filelist.each do |file|
            addition_path = ';"%RMX_HOME%' + file + '"'
            classpath += addition_path
        end
        classpath += ';"%RMX_HOME%slf4j-api-1.7.21.jar";"%RMX_HOME%slf4j-log4j12-1.7.21.jar";"%RMX_HOME%log4j-1.2.17.jar"'
        classpath
    end

    def build_classpath_linux(filelist)
        classpath = '"${RMX_HOME}.":"${RMX_HOME}translator.jar"'
        filelist.each do |file|
            addition_path = ':"${RMX_HOME}' + file + '"'
            classpath += addition_path
        end
        classpath += ':"${RMX_HOME}slf4j-api-1.7.21.jar":"${RMX_HOME}slf4j-log4j12-1.7.21.jar":"${RMX_HOME}log4j-1.2.17.jar"'
        classpath
    end

    def update_translator_sdk
        # In Disp_Root/Module/tonxlfile/bin, need to replace standard RMJavaSDK to FIPS RMJavaSDK library
        puts('Updating Dispatcher tonxlfile module SDK')
        dispatcher_tonxlfile = File.join(@fips_configuration.disp_root, 'Module', 'Translators', 'tonxlfile', 'bin')

        delete_files(dispatcher_tonxlfile, STANDARD_SDK_LIST)
        copy_files(@fips_sdk_folder, dispatcher_tonxlfile)

        # Replace translator.bat/sh by the generated file inside TcDRM folder during general FIPS SDK update above.
        generic_translator_folder = File.join(@fips_configuration.install_dir, 'TcDRM', 'Translator', 'tonxlfile', 'bin')

        FileUtils.cp_r(File.join(generic_translator_folder, 'translator_fips.bat'), File.join(dispatcher_tonxlfile, 'translator.bat'))

        FileUtils.cp_r(File.join(generic_translator_folder, 'translator_fips.sh'), File.join(dispatcher_tonxlfile, 'translator.sh'))
    end

    def stop_fcc
        puts('Killing any existing FCC process running for FIPSSDK deployment')
        fcc_kill = File.join(@fips_configuration.tc_root, 'tccs', 'bin', 'fccstat')
        fcc_kill = fcc_kill.tr('/', '\\') if RUBY_PLATFORM =~ /mswin|mingw|windows/
        fcc_kill_cmd = "\"#{fcc_kill}\" -kill | exit 0"
        call_system_command('Kill FCC process', 'fccstat', fcc_kill_cmd)
    end

    def start_fcc
        puts('Start FCC process')
        fcc_start = File.join(@fips_configuration.tc_root, 'tccs', 'bin', 'fccstat')
        fcc_start = fcc_start.tr('/', '\\') if RUBY_PLATFORM =~ /mswin|mingw|windows/
        fcc_start_cmd = "\"#{fcc_start}\" -start"
        call_system_command('start FCC process. Please check manually.', 'fccstat', fcc_start_cmd)
    end

    def update_partialdownload_sdk
        puts('Update PartialDownload Server Module SDK')
        tccs_folder = File.join(@fips_configuration.tc_root, 'tccs')
        nxl_library_folder = File.join(tccs_folder, 'third_party', 'nxl').tr('\\', '/')

        delete_files(nxl_library_folder, STANDARD_SDK_LIST)
        copy_files(@fips_sdk_folder, nxl_library_folder)

        puts('Remove standard SDK classpath from tccs_classpath.bat')
        tccs_classpath = File.join(@fips_configuration.tc_root, 'tccs', 'tccs_classpath.bat')
        delete_file_content_with_label(tccs_classpath, 'nextlabs partialdownload integration')

        jar_basenames = get_file_basenames_by_ext(nxl_library_folder, '*.jar')
        puts('Include NextLabs FIPS SDK to tccs_classpath.bat')
        if File.exist?(tccs_classpath)
            file = File.new(tccs_classpath, 'a+')
            file.puts "\n" unless file_endwith_new_line?(tccs_classpath)
            file.puts 'rem nextlabs partialdownload integration - start'
            jar_basenames.each do |jar_name|
                file.puts "set TCCS_CP=%TCCS_CP%;%FMS_HOME%\\third_party\\nxl\\#{jar_name}"
            end
            file.puts 'rem nextlabs partialdownload integration - end'
            file.close
        end
    end
end

updater = FIPSSDKUpdater.new
updater.update_sdk
