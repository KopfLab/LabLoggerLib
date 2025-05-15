# Guardfile
# to install: bundle install
# to run: bundle exec guard init rake

require 'yaml'

# constants
src_folder = "src"
bin_folder = "bin"

# program
last_bin = Dir.glob(File.join(bin_folder, '*.bin')).select { |f| File.file?(f) }.max_by { |f| File.mtime(f) }
program = File.basename(last_bin).split('-').first
puts "\nINFO: Setting up guard to re-compile '#{program}' when there are code changes in:"

# workflow
workflow_path = File.join(".github", "workflows", "compile-#{program}.yaml")
unless File.exist?(workflow_path)
    raise "Workflow YAML config file not found: #{workflow_path}"
end
workflow = YAML.load_file(workflow_path)

# watch paths
watch_paths = workflow.dig(true, "push", "paths").grep_v(/^\.github/) # note: "on" was interpreted as true
watch_paths.each do |path|
  puts " - #{path}"
end

# guard
guard :shell do
  watch(%r{^src/*\.cpp$}) do
    puts "Re-compiling bla"
  end
end
