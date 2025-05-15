# setup
require 'fileutils'
require 'yaml'

# recommended versions
# see https://docs.particle.io/reference/product-lifecycle/long-term-support-lts-releases/
versions = {
  'photon2' => '6.3.2', # not LTS but required for CloudEvent
  'photon' => '2.3.1',  # LTS
  'argon' => '4.2.0',   # LTS
  'boron' => '4.2.0'    # LTS
}

# constants
src_folder = "src"
lib_folder = "lib"
bin_folder = "bin"

# parameters
platform = ENV['PLATFORM'] || 'photon2'
version = ENV['VERSION'] || versions[platform]
device = ENV['DEVICE']
bin = ENV['BIN']

### PROGRAMS ###

task :blink => :compile
task :publish => :compile

### COMPILE ###

desc "compile binary in the cloud"
task :compile do
  # what program are we compiling?
  program = Rake.application.top_level_tasks.first

  # safety checks
  if platform.nil? || platform.strip.empty?
    raise "Error: PLATFORM must not be empty."
  end
  if version.nil? || version.strip.empty?
    raise "Error: VERSION must not be empty."
  end 

  # paths
  workflow_path = File.join(".github", "workflows", "compile-#{program}.yaml")
  unless File.exist?(workflow_path)
    raise "Workflow YAML config file not found: #{workflow_path}"
  end
  workflow = YAML.load_file(workflow_path)
  paths = workflow.dig("jobs", "compile", "strategy", "matrix", "program")[0]
  src_path = paths["src"]
  lib_path = paths["lib"]
  aux_path = paths["aux"]
  if src_path.nil? || src_path.strip.empty?
    raise "Error: could not extract src/lib/aux dependencies from #{workflow_path}"
  end 

  # source
  src_path = File.join(src_folder, src_path)
  unless Dir.exist?(src_path)
    raise "Error: folder '#{src_path}' does not exist."
  end

  # libs
  unless lib_path.nil? || lib_path.strip.empty?
    paths = lib_path.strip.split(/\s+/).map do |path|
      if Dir.exist?(path)
        File.join(path, src_folder)
      elsif Dir.exists?(File.join(lib_folder, path)) 
        File.join(lib_folder, path, src_folder)
      else
        raise "Warning: could not find '#{path}' library in root or #{lib_folder} - make sure it exists"
      end
    end.compact
    lib_path = paths.join(' ')
  end

  # info
  puts "\nINFO: compiling '#{program}' in the cloud for #{platform} #{version}...."
  puts " - src path: #{src_path}"
  puts " - lib paths: #{lib_path}"
  puts " - aux files: #{aux_path}"
  puts "\n"

  # compile
  sh "particle compile #{platform} #{src_path} #{src_path}/project.properties #{lib_path} #{aux_path} --target #{version} --saveTo #{bin_folder}/#{program}-#{platform}-#{version}.bin", verbose: false
end

### FLASH ###

desc "flash binary over the air or via usb"
task :flash do

  # is a binary selected?
  unless bin.nil? || bin.strip.empty?
    # user provided
    bin_path = File.join(bin_folder, bin)
  else
    # find latest
    files = Dir.glob(File.join(bin_folder, '*.bin')).select { |f| File.file?(f) }
    if files.empty?
      raise "No .bin files found in #{bin_folder}"
    end
    bin_path = files.max_by { |f| File.mtime(f) }
  end

  # safety check
  unless File.exist?(bin_path)
    raise "Binary file does not exit: #{bin_path}"
  end

  # OTA or serial?
  unless device.nil? || device.strip.empty?
    # over the air
    puts "\nINFO: flashing #{bin_path} to #{device} via the cloud..."
    sh "particle flash #{device} #{bin_path}"
  else
    puts "\nINFO: putting device into DFU mode"
    sh "particle usb dfu"
    puts "INFO: flashing #{bin_path} over USB (requires device in DFU mode = yellow blinking)..."
    sh "particle flash --usb #{bin_path}"
  end
end

### TOOLS ###

desc "list available devices connected to USB"
task :list do
  puts "\nINFO: querying list of available USB devices..."
  sh "particle usb list"
end

desc "get MAC address of device connected to USB"
task :mac do
  puts "\nINFO: checking for MAC address...."
  sh "particle serial mac"
end

desc "start serial monitor"
task :monitor do
  puts "\nINFO: connecting to serial monitor..."
  sh "particle serial monitor --follow"
end
