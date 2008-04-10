require 'rake/gempackagetask'

WIN_SUFFIX = ENV['WIN_SUFFIX'] || 'i386-mswin32'

task :clean => :clobber_package

spec = Gem::Specification.new do |s|
  s.name                  = 'thin-turbo'
  s.version               = THIN_TURBO_VERSION
  s.platform              = WIN ? Gem::Platform::CURRENT : Gem::Platform::RUBY
  s.summary               = 
  s.description           = "New backend for Thin web server"
  s.author                = "Marc-Andre Cournoyer"
  s.email                 = 'macournoyer@gmail.com'
  s.homepage              = 'http://code.macournoyer.com/thin/'
  s.rubyforge_project     = 'thin'
  s.has_rdoc              = false

  s.required_ruby_version = '>= 1.8.5'
  
  s.add_dependency        'thin', '>= 0.8.0'

  s.files                 = %w(COPYING README Rakefile) +
                            Dir.glob("{lib,test,tasks}/**/*") + 
                            Dir.glob("ext/**/*.{h,c,rb,rl}")
  
  if WIN
    s.files              += ["lib/thin_backend.#{Config::CONFIG['DLEXT']}"]
  else
    s.extensions          = FileList["ext/**/extconf.rb"].to_a
  end
  
  s.require_path          = "lib"
end

Rake::GemPackageTask.new(spec) do |p|
  p.gem_spec = spec
end

namespace :gem do
  desc 'Upload gem to code.macournoyer.com'
  task :upload => :gem do
    upload "pkg/#{spec.full_name}.gem", 'gems'
    system 'ssh macournoyer@macournoyer.com "cd code.macournoyer.com && gem generate_index"'
  end
  
  namespace :upload do
    desc 'Upload the precompiled win32 gem to code.macournoyer.com'
    task :win do
      upload "pkg/#{spec.full_name}-#{WIN_SUFFIX}.gem", 'gems'
      system 'ssh macournoyer@macournoyer.com "cd code.macournoyer.com && gem generate_index"'
    end    

    desc 'Upload gems (ruby & win32) to rubyforge.org'
    task :rubyforge => :gem do
      sh 'rubyforge login'
      sh "rubyforge add_release thin thin-turbo #{THIN_TURBO_VERSION} pkg/#{spec.full_name}.gem"
      sh "rubyforge add_file thin thin-turbo #{THIN_TURBO_VERSION} pkg/#{spec.full_name}.gem"
      sh "rubyforge add_file thin thin-turbo #{THIN_TURBO_VERSION} pkg/#{spec.full_name}-#{WIN_SUFFIX}.gem"
    end
  end
end

task :install => [:clobber, :compile, :package] do
  sh "#{SUDO} gem install pkg/#{spec.full_name}.gem"
end

task :uninstall => :clean do
  sh "#{SUDO} gem uninstall -v #{THIN_TURBO_VERSION} -x thin-turbo"
end
