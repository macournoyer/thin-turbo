require 'thin_backend'

module Thin
  class Backend
    def start
      listen
      puts 'Listening on 0.0.0.0:4000'

      trap('INT') { stop }

      @running = true
      loop! while @running
      close
    end

    def stop
      puts 'Stopping ...'
      @running = false
    end

    def log_error(ex=$!)
      puts "!! Unexpected error while processing request: #{ex.message}"
      puts ex.backtrace.join("\n") if ex.backtrace
    end
  end
end