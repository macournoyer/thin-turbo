class App
  BODY = <<-EOS
    <html>
    <h1>Hi there!</h1>
    <p>Type something ...</p>
    <form action="/" method="post">
      <textarea name="name">Type somethng ...</textarea><br/>
      <input type="submit" />
      <a href="/">Cancel</a>
    </form>
    </html>
  EOS
  
  SIZE = BODY.size

  def call(env)
    sleep 10 if env['PATH_INFO'] == '/slow'
    
    puts env['rack.input'].read if env['CONTENT_LENGTH'].to_i > 0
    [
      200,
      {
        'Content-Type' => 'text/html',
        'Content-Length' => SIZE.to_s
      },
      BODY
    ]
  end
end

run App.new
