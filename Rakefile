task :default => :test

require 'rake/clean'
CLEAN.include '*.plist', '*.gch'

desc 'Startup an IRb console with screen_recorder loaded'
task :console => [:compile] do
  sh 'irb -Ilib -rscreen_recorder'
end

require 'rake/testtask'
Rake::TestTask.new do |t|
  t.libs << '.'
  t.pattern = 'test/*_test.rb'
end
task :test => :compile


# Gem stuff

require 'rubygems/package_task'
SPEC = Gem::Specification.load('screen_recorder.gemspec')

Gem::PackageTask.new(SPEC) { }

desc 'Build and install gem (not including deps)'
task :install => :gem do
  require 'rubygems/installer'
  Gem::Installer.new("pkg/#{SPEC.file_name}").install
end

require 'rake/extensiontask'
Rake::ExtensionTask.new('screen_recorder', SPEC)
