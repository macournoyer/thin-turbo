require File.dirname(__FILE__) + '/spec_helper'

describe Thin::Backends::Turbo, "parsing" do
  serve EchoApp.new
  
  it "should parse" do
    GET("/path?query")
    
    @app.env['PATH_INFO'].should == '/path'
    @app.env['REQUEST_PATH'].should == '/path'
    @app.env['REQUEST_URI'].should == '/path?query'
    @app.env['HTTP_VERSION'].should == 'HTTP/1.1'
    @app.env['REQUEST_METHOD'].should == 'GET'
    @app.env['QUERY_STRING'].should == 'query'
    @app.env['HTTP_CONNECTION'].should == 'close'
    @app.env['SERVER_PROTOCOL'].should == 'HTTP/1.1'
    @app.env['GATEWAY_INTERFACE'].should == 'CGI/1.2'
    @app.env["rack.url_scheme"].should == 'http'
    @app.env['FRAGMENT'].should be_nil
    @app.env['REMOTE_ADDR'].should == '127.0.0.1'
  end
end
