class App
  def call(env)
    body = env['PATH_INFO'] + env['rack.input'].read.to_s
    [200, { 'Content-Type' => 'text/html', 'Content-Length' => body.size.to_s }, body]
  end
end