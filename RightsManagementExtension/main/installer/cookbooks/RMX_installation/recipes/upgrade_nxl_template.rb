# Teamcenter RMX 5.1
# Cookbook Name :: RMX_installation
# Recipes       :: upgrade_nxl_template
# Description   :: Upgrade nextlabs template, import new tc preferences
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

require 'fileutils'
Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'upgrade_nextlabs_template' do

    def extract_template_information(nextlabs_template_name, nxl_data_model)
        log_info("Copy feature_#{nextlabs_template_name}.xml and #{nextlabs_template_name}Bundle_en_US.xml to TC install modules folder")
        feature_nextlabs_src = File.join(nxl_data_model, "feature_#{nextlabs_template_name}.xml")
        feature_nextlabs_des = File.join(node['TC_ROOT'], 'install', 'install', 'modules')
        FileUtils.cp_r(feature_nextlabs_src, feature_nextlabs_des)

        nextlabs_bundle_src = File.join(nxl_data_model, "#{nextlabs_template_name}Bundle_en_US.xml")
        nextlabs_bundle_des = File.join(node['TC_ROOT'], 'install', 'install', 'lang', 'en_US')
        FileUtils.cp_r(nextlabs_bundle_src, nextlabs_bundle_des)

        log_info("Unzip #{nextlabs_template_name}_template.zip and #{nextlabs_template_name}_install.zip")
        nextlabs_template_zip = File.join(nxl_data_model, "#{nextlabs_template_name}_template.zip")
        nextlabs_install_zip = File.join(nxl_data_model, "#{nextlabs_template_name}_install.zip")
        extract_zip(nextlabs_template_zip, node['TC_ROOT'])
        extract_zip(nextlabs_install_zip, node['TC_ROOT'])

        log_info("Copy client_#{nextlabs_template_name}.properties and #{nextlabs_template_name}_icons.zip to #{nextlabs_template_name} folder")
        tc_root_nextlabs = File.join(node['TC_ROOT'], 'install', nextlabs_template_name)
        client_nextlabs_src = File.join(nxl_data_model, "client_#{nextlabs_template_name}.properties")
        nextlabs_icons_src = File.join(nxl_data_model, "#{nextlabs_template_name}_icons.zip")
        FileUtils.cp_r(client_nextlabs_src, tc_root_nextlabs)
        FileUtils.cp_r(nextlabs_icons_src, tc_root_nextlabs)
    end

    def deploy_nextlabs_template
        # List of template to customize. (Server)
        # NEXTLABS_CUTOMIZED_TEMPLATE = %w(nxl_template nxl_ipem_template nxl_swim_template)
        template_list = []
        node['feature_list'].each do |feature_name|
            if feature_name == 'nxl_template'
                template_list.push('nextlabs')
            elsif feature_name == 'nxl_ipem_template'
                template_list.push('nextlabsipem')
            elsif feature_name == 'nxl_swim_template'
                template_list.push('nextlabsswim')
            end
        end
        template_list_str = template_list.join(',')

        log_info("NextLabs Template will be deploy: #{template_list_str}")

        install_runner = File.join(node['TC_ROOT'], 'bin', Script::INSTALL_RUNNER)
        bmide_generatetcplmxmlschema = File.join(node['TC_ROOT'], 'bin', Script::BMIDE_GENERATETCPLMXMLSCHEMA)
        bmide_setupknowledgebase = File.join(node['TC_ROOT'], 'bin', Script::BMIDE_SETUPKNOWLEDGEBASE)
        bmide_modeltool = File.join(node['TC_ROOT'], 'bin', Script::BMIDE_MODELTOOL)
        model_file = File.join(node['TC_DATA'], 'model', 'model.xml')

        if RUBY_PLATFORM =~ /mswin|mingw|windows/
            install_runner = install_runner.tr('/', '\\')
            bmide_generatetcplmxmlschema = bmide_generatetcplmxmlschema.tr('/', '\\')
            bmide_setupknowledgebase = bmide_setupknowledgebase.tr('/', '\\')
            bmide_modeltool = bmide_modeltool.tr('/', '\\')
            model_file = model_file.tr('/', '\\')
        end

        # install commands
        # path contains spaces, have to use double-quote
        # due to encoding issue in Teamcenter 11.2.3, do not ruby to call TC utilities directly
        log_info('Run Install Command')
        step = 'Install NextLabs Customized Template'
        install_runner_cmd = "#{node['tc_profilevar']} && \"#{install_runner}\" -u=infodba -p=#{node['infodba_psd']} -g=dba -templates=#{template_list_str}"
        # For some reason, call_system_command can't return all logs should be output to Chef log, which might confuse developer which template are deploying?
        # Using quick system() function will output all log...
        ret = system(install_runner_cmd)
        raise "Command Install Runner failed when try to deploy multiple nextlabs templates" unless ret
        # call_system_command(step, 'install_runner', install_runner_cmd, :no)
        bmide_generatetcplmxmlschema_cmd = "#{node['tc_profilevar']} && \"#{bmide_generatetcplmxmlschema}\" -u=infodba -p=#{node['infodba_psd']} -g=dba"
        call_system_command(step, 'bmide_generatetcplmxmlschema', bmide_generatetcplmxmlschema_cmd, :no)
        bmide_setupknowledgebase_cmd = "#{node['tc_profilevar']} && \"#{bmide_setupknowledgebase}\" -u=infodba -p=#{node['infodba_psd']} -g=dba -regen=false"
        call_system_command(step, 'bmide_setupknowledgebase', bmide_setupknowledgebase_cmd, :no)
        bmide_modeltool_cmd = "#{node['tc_profilevar']} && \"#{bmide_modeltool}\" -u=infodba -p=#{node['infodba_psd']} -g=dba -tool=all -mode=install -target_dir=\"#{ENV['TC_DATA']}\" -model_file=\"#{model_file}\""
        call_system_command(step, 'bmide_modeltool', bmide_modeltool_cmd, :no)
    end

    block do
        log_info("Will redeloy template? : #{ENV['template_upgrade']}")
        if !ENV['template_upgrade'].nil? && ENV['template_upgrade'] == 'yes'
            log_info(LOGMSG::UPGRADE_NXL_TEMPLATE)
            # in case of accidental break
            set_break_point(node['temp_dir'], recipe_name)

            if node['feature_list'].include?('nxl_template')
                template_name = 'nextlabs'
                if node['tc_version'] == '10'
                    nxl_data_model = File.join(ENV['RMX_ROOT'], 'DataModel', 'nextlabs')
                elsif node['tc_version'] == '11'
                    nxl_data_model = File.join(ENV['RMX_ROOT'], 'DataModel', 'nextlabs_tc11')
                else
                    nxl_data_model = File.join(ENV['RMX_ROOT'], 'DataModel', 'nextlabs_tc12')
                end
                extract_template_information(template_name, nxl_data_model)
            end

            if node['feature_list'].include?('nxl_ipem_template')
                template_name = 'nextlabsipem'
                if node['tc_version'] == '10'
                    nxl_ipem_data_model = File.join(ENV['RMX_ROOT'], 'DataModel', 'nextlabs_ipem', 'tc10')
                elsif node['tc_version'] == '11'
                    nxl_ipem_data_model = File.join(ENV['RMX_ROOT'], 'DataModel', 'nextlabs_ipem', 'tc11')
                else
                    nxl_ipem_data_model = File.join(ENV['RMX_ROOT'], 'DataModel', 'nextlabs_ipem', 'tc12')
                end
                extract_template_information(template_name, nxl_ipem_data_model)
            end

            if node['feature_list'].include?('nxl_swim_template')
                template_name = 'nextlabsswim'
                if node['tc_version'] == '11'
                    nxl_swim_data_model = File.join(ENV['RMX_ROOT'], 'DataModel', 'nextlabs_swim', 'tc11')
                else
                    nxl_swim_data_model = File.join(ENV['RMX_ROOT'], 'DataModel', 'nextlabs_swim', 'tc12')
                end
                extract_template_information(template_name, nxl_swim_data_model)
            end

            deploy_nextlabs_template()
        end
    end
end

ruby_block 'import_new_tc_preferences' do
    block do
        if node['feature_list'].include?('nxl_template')
            log_info('[Progress] Importing NextLabs Teamcenter preferences...')
            # modify nextlabs preference
            nxl_preference_template = File.join(ENV['RMX_ROOT'], 'TcPreferences', 'NXL_TcPreferences_Template.xml')
            file = File.new(nxl_preference_template)
            doc = REXML::Document.new(file)

            preference_elements = doc.elements.to_a('preferences/category/preference')
            preference_elements.each do |preference|
                # if preference.attributes['name'].eql?('NXL_Remote_IP')
                #     preference.elements.to_a('context/value')[0].text = node['jpc_remote_ip']
                # end
                if preference.attributes['name'].eql?('NXL_TCSOA_Hostname')
                    # preference.elements.to_a('context/value')[0].text = node['tcsoa_hostname']
                    preference.elements.to_a('context/value')[0].text = "SOA_HOST"
                end
                if preference.attributes['name'].eql?('NXL_RMX_Solution')
                    value = 'AtRest' # if node['solution_type'].eql?('Protection in Teamcenter volume')
                    value = 'InMotion' if node['solution_type'].eql?('Protection on Download')
                    preference.elements.to_a('context/value')[0].text = value
                end
                if preference.attributes['name'].eql?('NXL_PDP_DEFAULT_ACTION')
                    preference.elements.to_a('context/value')[0].text = 'deny'
                end

                if preference.attributes['name'].eql?('NXL_PDPHOST_SERVERAPP')
                    preference.elements.to_a('context/value')[0].text = node['jpc_remote_ip']
                end
                if preference.attributes['name'].eql?('NXL_PDPPORT_SERVERAPP')
                    preference.elements.to_a('context/value')[0].text = node['jpc_eva_port']
                end
                if preference.attributes['name'].eql?('NXL_PDP_HTTPS_SERVERAPP')
                    preference.elements.to_a('context/value')[0].text = node['jpc_https']
                end
                if preference.attributes['name'].eql?('NXL_OAUTH2HOST')
                    preference.elements.to_a('context/value')[0].text = node['cc_host']
                end
                if preference.attributes['name'].eql?('NXL_OAUTH2PORT')
                    preference.elements.to_a('context/value')[0].text = node['cc_port']
                end
                if preference.attributes['name'].eql?('NXL_OAUTH2_HTTPS')
                    preference.elements.to_a('context/value')[0].text = node['cc_oauth2_https']
                end
                if preference.attributes['name'].eql?('NXL_OAUTH2_CLIENTID')
                    preference.elements.to_a('context/value')[0].text = node['cc_apiuser']
                end
                if preference.attributes['name'].eql?('NXL_OAUTH2_CLIENTSECRET')
                    preference.elements.to_a('context/value')[0].text = node['cc_apisecret']
                end
                if preference.attributes['name'].eql?('NXL_PDP_IGNOREHTTPS_SERVERAPP')
                    preference.elements.to_a('context/value')[0].text = node['cc_ignore_https_cert']
                end
            end

            nxl_tc_preference = File.join(ENV['RMX_ROOT'], 'TcPreferences', 'NXL_TcPreferences.xml')
            formatter = REXML::Formatters::Pretty.new(2)
            formatter.compact = true
            File.open(nxl_tc_preference, 'w') do |f|
                # f.puts doc
                formatter.write(doc, f)
            end

            nxl_tc_preference_merge = File.join(ENV['RMX_ROOT'], 'TcPreferences', 'NXL_TcPreferences.merge.xml')
            nxl_tc_preference_merge = nxl_tc_preference_merge.tr('/', '\\') if RUBY_PLATFORM =~ /mswin|mingw|windows/
            # Perferences_manager will update a log, have to use \dir\subdir\ style, cannot use /dir/subdir
            manager = File.join(node['TC_ROOT'], 'bin', TeamcenterProcess::PREFERENCES_MANAGER)
            manager = manager.tr('/', '\\') if RUBY_PLATFORM =~ /mswin|mingw|windows/
            nxl_tc_preference = nxl_tc_preference.tr('/', '\\') if RUBY_PLATFORM =~ /mswin|mingw|windows/
            step = 'Install NXL Template'
            preference_manager_cmd = "#{node['tc_profilevar']} && \"#{manager}\" -mode=import -scope=SITE -file=\"#{nxl_tc_preference}\" -u=infodba -p=#{node['infodba_psd']} -g=dba -action=OVERRIDE && \"#{manager}\" -mode=import -scope=SITE -file=\"#{nxl_tc_preference_merge}\" -u=infodba -p=#{node['infodba_psd']} -g=dba -action=MERGE"
            call_system_command(step, 'preference_manager', preference_manager_cmd, :no)

            # if installed Active Workspace runtime server
            # need to import aws preference: cellproperties_tool_used, AWC_StartupPreferences
            if node['aws_runtime']
                cellproperties_tool_used = File.join(ENV['RMX_ROOT'], 'NXLOverlay', 'cellproperties_tool_used.xml')
                exe_cmd = "#{node['tc_profilevar']} && \"#{manager}\" -mode=import -scope=SITE -file=\"#{cellproperties_tool_used}\" -u=infodba -p=#{node['infodba_psd']} -g=dba -action=MERGE"
                call_system_command('Install AWS Support', TeamcenterProcess::PREFERENCES_MANAGER, exe_cmd, :no)

                awc_startup_preferences = File.join(ENV['RMX_ROOT'], 'TcPreferences', 'AWC_StartupPreferences.xml')
                exe_cmd = "#{node['tc_profilevar']} && \"#{manager}\" -mode=import -scope=SITE -file=\"#{awc_startup_preferences}\" -u=infodba -p=#{node['infodba_psd']} -g=dba -action=MERGE"
                call_system_command('Install AWS Support', TeamcenterProcess::PREFERENCES_MANAGER, exe_cmd, :no)
            end
        end
    end
end

