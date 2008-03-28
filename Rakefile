RUBY_1_9 = RUBY_VERSION =~ /^1\.9/
WIN      = (RUBY_PLATFORM =~ /mswin|cygwin/)
SUDO     = (WIN ? "" : "sudo")

require 'rake'
require 'rake/clean'
require 'lib/thin'

Dir['tasks/**/*.rake'].each { |rake| load rake }

task :default => :spec

ext_task :thin_parser
ext_task :thin_backend

ragel_task 'ext/thin_backend', 'parser.rl', 'parser.c'

task :test do
  cd 'test' do
    sh 'make test'
  end
end
