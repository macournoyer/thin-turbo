require 'thin_backend'

b = Thin::Backend.new('0.0.0.0', 3000)
trap('INT') do
  puts 'Stopping ...'
  b.stop
end
puts 'Listening on 0.0.0.0:3000'
b.start