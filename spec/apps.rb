class EchoApp
  def call(env)
    if env['REQUEST_METHOD'] == 'GET'
      body = env['REQUEST_URI']
    else
      body = env['rack.input'].read
    end
    [200, { 'Content-Type' => 'text/html', 'Content-Length' => body.size.to_s }, body]
  end
end