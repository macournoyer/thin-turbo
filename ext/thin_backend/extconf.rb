require 'mkmf'

flags = []

if have_header('sys/select.h')
  flags << '-DEV_USE_SELECT'
end

if have_header('poll.h')
  flags << '-DEV_USE_POLL'
end

if have_header('sys/epoll.h')
  flags << '-DEV_USE_EPOLL'
end

if have_header('sys/event.h') and have_header('sys/queue.h')
  flags << '-DEV_USE_KQUEUE'
end

if have_header('port.h')
  flags << '-DEV_USE_PORT'
end

if have_header('sys/inotify.h')
  flags << '-DEV_USE_INOTIFY'
end

dir = File.dirname(__FILE__)
libev_dir = File.expand_path(dir + '/../libev')

$CFLAGS << " -I#{libev_dir} " << flags.join(' ')

$defs << "-DRUBY_19" if RUBY_VERSION =~ /^1\.9/

$defs << "-DDEBUG" if ENV["DEBUG"]

dir_config("thin_backend")
have_library("c", "main")

create_makefile("thin_backend")
