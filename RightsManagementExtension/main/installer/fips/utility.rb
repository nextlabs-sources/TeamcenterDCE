require 'fileutils'
require 'open3'

module Utility
    # Delete files contained in filenamses inside a directory
    def delete_files(dir, filenames)
        filenames.each do |file|
            FileUtils.remove_file(File.join(dir, file), true)
        end
    end

    # copy files from src_dir to des_dir
    def copy_files(src_dir, des_dir)
        FileUtils.mkdir_p(des_dir) unless File.exist?(des_dir)
        files = File.join(src_dir, '*')
        Dir[files].each do |file|
            FileUtils.cp_r(file, des_dir)
        end
    end

    # get all the basenames of files inside a directory
    def get_file_basenames(dir)
        filenames = []
        files = File.join(dir, '*')
        Dir[files].each do |file|
            name = File.basename(file)
            filenames.push(name)
        end
        filenames
    end

    # get all the basenames of files inside a directory by extension
    # ext: *.jar
    def get_file_basenames_by_ext(dir, ext)
        filenames = []
        files = File.join(dir, ext)
        Dir[files].each do |file|
            name = File.basename(file)
            filenames.push(name)
        end
        filenames
    end

    # Delete line content between given label (Assume only 2 label enclose lines to delete)
    def delete_file_content_with_label(filepath, label)
        file = File.new(filepath)
        lines = file.readlines
        start_line = -1         # The first line that found label
        end_line = -1           # The second (last) line that found label
        (0...lines.length).each do |i|
            next unless lines[i].include?(label)

            if start_line == -1
                start_line = i          # Update start_line if include label
            else
                end_line = i            # Update end_line if found label again
            end
        end
        return if start_line == -1        # Ignore if not found label in the file
        end_line = start_line if end_line == -1     # Update end_line if found label only once (should not happen)
        file.close
        lines.slice!(start_line, end_line - start_line + 1)

        file = File.new(filepath, 'w')
        file.puts(lines)
        file.close
    end

    # Check if this file is ended with new line
    def file_endwith_new_line?(filepath)
        file = File.new(filepath)
        lines = file.readlines
        file.close
        lines[lines.length - 1].end_with?("\n")
    end

    # New call_system_command function
    # Should provide error stack trace as well on error/exception happens
    # https://blog.bigbinary.com/2012/10/18/backtick-system-exec-in-ruby.html
    # Using popen2e will merge stdout and stderr to same stream. We output to RMX log all of them
    def call_system_command(step, cmd_name, cmd, full_log = :yes)
        Open3.popen2e(cmd) do |_stdin, stdout_err, wait_thr|
            while stdout_err.gets
                # Full log will include Chef format timestamp inside log file.
                if full_log == :yes
                    puts(stdout_err.gets)
                else # Some Teamcenter utility command return alot of logs and in long line. This way, we can strip the Chef timestamp part.
                    puts(" #{stdout_err.gets}")
                end
            end

            exit_status = wait_thr.value
            unless exit_status.success?
                raise "Command #{cmd_name} failed when try to #{step}"
            end
        end
    end
end