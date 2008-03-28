require 'thin_backend'

class Thin::Backend 
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

app = proc do |env|
  # body = ['hello', ' world!']
  body = 'hello world!'
  [
    200,
    {
      'Content-Type' => 'text/html',
      'Content-Length' => body.size.to_s
    },
    body
  ]
end

b = Thin::Backend.new('0.0.0.0', 4000, app)

b.start
