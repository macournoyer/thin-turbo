require 'rubygems'
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

body = DATA.read

app = proc do |env|
  puts env['rack.input'].read if env['CONTENT_LENGTH'].to_i > 0
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

__END__
<html>
<h1>Hi there!</h1>
<p>Type something ...</p>
<form action="?" method="post">
  <input type="text" name="name" value="value" />
  <input type="submit" />
  <a href="/">Cancel</a>
</form>
</html>