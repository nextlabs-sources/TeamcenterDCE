# Teamcenter RMX 5.2
# Cookbook Name :: RMX_installation
# Recipes       :: uninstall_foundation_customization
# Description   :: Restore nextlabs cutomized template - foundation, ipem, swim
# Author        :: Phan Anh Tuan
# Copyright 2019, Nextlabs Inc.

Chef::Resource::RubyBlock.send(:include, Utility)

ruby_block 'restore_foundation_customization' do

    def restore_template(template_name, sample_template)
        log_info("Restore original #{template_name} template...")
        template_modifier = File.join(ENV['RMX_ROOT'], 'TemplateModifier', Script::TEMPLATE_MODIFIER)

        system("chmod +x \"#{template_modifier}\"") if RUBY_PLATFORM =~ /linux/ # in case .sh have no execution right
        templatemod_cmd = "\"#{template_modifier}\" -a undo -t #{template_name} -i \"#{sample_template}\""

        call_system_command("Restore original #{template_name} template", 'templatemod', templatemod_cmd, :no)
    end    

    block do
        log_info(LOGMSG::UNINSTALL_FOUNDATION_CUSTOMIZATION)
        # in case of accidental break
        set_break_point(node['temp_dir'], recipe_name)

        template_list = []
        node['feature_list'].each do |feature_name|
            if feature_name == 'nxl_template'
                template_list.push('foundation')
                sample_template = File.join(ENV['RMX_ROOT'], 'TemplateModifier', 'sampleFoundationInput.xml')
                restore_template('foundation', sample_template)
            elsif feature_name == 'nxl_ipem_template'
                template_list.push('ipem')
                ipem_sample_template = File.join(ENV['RMX_ROOT'], 'TemplateModifier', 'sampleIpemInput.xml')
                restore_template('ipem', ipem_sample_template)
            elsif feature_name == 'nxl_swim_template'
                template_list.push('swim')
                swim_sample_template = File.join(ENV['RMX_ROOT'], 'TemplateModifier', 'sampleSwimInput.xml')
                restore_template('swim', swim_sample_template)
            end
        end
        template_list_str = template_list.join(',')

        # Restore original template
        bmide_template = File.join(node['TC_ROOT'], 'bin', Script::BMIDE_PROCESSTEMPLATES)
        model_dir = File.join(node['TC_DATA'], 'model')
        if RUBY_PLATFORM =~ /mswin|mingw|windows/
            bmide_template = bmide_template.tr('/', '\\')
            model_dir = model_dir.tr('/', '\\')
        end
        cmd = "#{node['tc_profilevar']} && \"#{bmide_template}\" -u=infodba -p=#{node['infodba_psd']} -g=dba -mode=server -dir=\"#{model_dir}\" -templates=#{template_list_str}"
        call_system_command('Redeploy original Foundation template', 'bmide_processtemplates', cmd, :no)
    end
end
