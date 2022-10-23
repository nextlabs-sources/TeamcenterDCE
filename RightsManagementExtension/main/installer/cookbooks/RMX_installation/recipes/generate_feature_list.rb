require 'fileutils'

# provide a txt file, which contains all the features to be installed, for Trista's auto check scripts

begin
    path = File.join(ENV['START_DIR'], 'featureList.txt')
    file = File.new(path, 'w+')
    node['feature_list'].each do |feature|
        file.puts feature
    end
    file.close
end
