RUBY_1_9 = RUBY_VERSION =~ /^1\.9/
WIN      = (RUBY_PLATFORM =~ /mswin|cygwin/)
SUDO     = (WIN ? "" : "sudo")
THIN_TURBO_VERSION  = '0.1.0'

require 'rake'
require 'rake/clean'
require 'spec/rake/spectask' unless RUBY_1_9

Dir['tasks/**/*.rake'].each { |rake| load rake }

task :default => [:compile, :test, :spec]

ext_task :thin_backend

ragel_task 'ext/thin_backend', 'parser.rl', 'parser.c'

task :test do
  cd 'test' do
    sh 'make test'
  end
end

Spec::Rake::SpecTask.new(:spec) do |t|
  t.spec_opts = %w(-fs -c)
  t.spec_files = FileList['spec/**/*_spec.rb']
end unless RUBY_1_9