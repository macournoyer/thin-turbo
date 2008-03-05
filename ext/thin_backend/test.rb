require 'thin_backend'

class Thin::Backend
  def process(env)
    [200, {}, 'ok']
  end
end

b = Thin::Backend.new('0.0.0.0', 4000)

puts 'Listening on 0.0.0.0:4000'
b.start

b.loop!

puts 'Stopping ...'
b.stop
