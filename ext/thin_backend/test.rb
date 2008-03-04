require 'thin_backend'

b = Thin::Backend.new('0.0.0.0', 3000)
trap('INT') do
  puts 'Received INT signal'
  @running = false
end

puts 'Listening on 0.0.0.0:3000'
@running = true
b.start

while @running
  b.process
end

puts 'Stopping ...'
b.stop
