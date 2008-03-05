require 'thin_backend'

class Thin::Backend 
  def start
    listen
    puts 'Listening on 0.0.0.0:4000'
    @running = true
    loop! while @running
    close
  end
  
  def stop
    puts 'Stopping ...'
    @running = false
  end
  
  def process(env)
    [200, {}, 'ok']
  end
end

b = Thin::Backend.new('0.0.0.0', 4000)

trap('INT') do
  b.stop
end

b.start
