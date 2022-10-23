# Teamcenter RMX 5.0
# Cookbook Name :: RMX_installation
# Recipes       :: install_foundation_customization
# Description   :: Customize Teamcenter foundation template
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'install_foundation_customization' do

    def customize_template(template_name, sample_template)
        log_info("Modifying #{template_name} template...")
        template_modifier = File.join(ENV['RMX_ROOT'], 'TemplateModifier', Script::TEMPLATE_MODIFIER)

        system("chmod +x \"#{template_modifier}\"") if RUBY_PLATFORM =~ /linux/ # in case .sh have no execution right
        templatemod_cmd = "\"#{template_modifier}\" -a customize -t #{template_name} -i \"#{sample_template}\""

        call_system_command("Customize #{template_name} template", 'templatemod', templatemod_cmd, :no)
    end

    block do
        log_info(LOGMSG::INSTALL_FOUNDATION_CUSTOMIZATION)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)

        template_list = []
        node['feature_list'].each do |feature_name|
            if feature_name == 'nxl_template'
                template_list.push('foundation')
                sample_template = File.join(ENV['RMX_ROOT'], 'TemplateModifier', 'sampleFoundationInput.xml')
                customize_template('foundation', sample_template)
            elsif feature_name == 'nxl_ipem_template'
                template_list.push('ipem')
                ipem_sample_template = File.join(ENV['RMX_ROOT'], 'TemplateModifier', 'sampleIpemInput.xml')
                customize_template('ipem', ipem_sample_template)
            elsif feature_name == 'nxl_swim_template'
                template_list.push('swim')
                swim_sample_template = File.join(ENV['RMX_ROOT'], 'TemplateModifier', 'sampleSwimInput.xml')
                customize_template('swim', swim_sample_template)
            end
        end
        template_list_str = template_list.join(',')

        # Deploy customized foundation template
        log_info('Redeploying customized template into database...')
        bmide_template = File.join(node['TC_ROOT'], 'bin', Script::BMIDE_PROCESSTEMPLATES)
        model_dir = File.join(node['TC_DATA'], 'model')
        if RUBY_PLATFORM =~ /mswin|mingw|windows/
            bmide_template = bmide_template.tr('/', '\\')
            model_dir = model_dir.tr('/', '\\')
        end
        cmd = "#{node['tc_profilevar']} && \"#{bmide_template}\" -u=infodba -p=#{node['infodba_psd']} -g=dba -mode=server -dir=\"#{model_dir}\" -templates=#{template_list_str}"
        call_system_command('Redeploy customized template into database', 'bmide_processtemplates', cmd, :no)

        # generate_metadata_cache will create a syslog file under temp folder, have to use its basename
        # otherwise, there will be a problem
        generate_metadata_cache = TeamcenterProcess::GENERATE_METADATA_CACHE
        generate_client_meta_cache = TeamcenterProcess::GENERATE_CLIENT_META_CACHE
        bmide_generate_client_cache = TeamcenterProcess::BMIDE_GENERATE_CLIENT_CACHE
        binfolder = File.join(node['TC_ROOT'], 'bin')
        model_file = File.join(node['TC_DATA'], 'model', 'model.xml')
        if RUBY_PLATFORM =~ /mswin|mingw|windows/
            binfolder = binfolder.tr('/', '\\')
            model_file = model_file.tr('/', '\\')
            generate_metadata_cache = generate_metadata_cache.tr('/', '\\')
            generate_client_meta_cache = generate_client_meta_cache.tr('/', '\\')
            bmide_generate_client_cache = bmide_generate_client_cache.tr('/', '\\')
        end
        if (node['tc_full_version'][0, 3] >= '132') # generate_client_meta_cache deprecated in TC 13.2
            log_info('Generate Metadata and Client Cache (TC 13.2+)')
            exe_cmd = "#{node['tc_profilevar']} && #{generate_metadata_cache} -u=infodba -p=#{node['infodba_psd']} -g=dba && #{bmide_generate_client_cache} -mode=generate -cache=all -model_file=\"#{model_file}\" -u=infodba -p=#{node['infodba_psd']} -g=dba"
        else
            log_info('Generate Metadata and Client Cache')
            exe_cmd = "#{node['tc_profilevar']} && #{generate_metadata_cache} -u=infodba -p=#{node['infodba_psd']} -g=dba && #{generate_client_meta_cache} generate all -u=infodba -p=#{node['infodba_psd']} -g=dba"
        end
        exe_cmd = "cd /D #{binfolder} && " + exe_cmd if RUBY_PLATFORM =~ /mswin|mingw|windows/
        exe_cmd = "cd #{binfolder} && " + exe_cmd if RUBY_PLATFORM =~ /linux/
        call_system_command('Generate Metadata and Client Cache', 'generate_metadata_cache', exe_cmd, :no)
    end
end
