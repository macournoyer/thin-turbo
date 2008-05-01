require 'rack/adapter/rails'

# Monkey patch Thin Rails adapter to add a mutex
module Rack
  module Adapter
    class Rails
      def initialize_with_mutex(*args)
        @mutex = Mutex.new
        initialize_without_mutex(*args)
        
        puts "using threaded rails"
      end
      alias_method :initialize_without_mutex, :initialize
      alias_method :initialize, :initialize_with_mutex
      
      def serve_rails(env)
        request         = Request.new(env)
        response        = Response.new
        
        session_options = ActionController::CgiRequest::DEFAULT_SESSION_OPTIONS
        cgi             = CGIWrapper.new(request, response)
    
        @mutex.synchronize do
          Dispatcher.dispatch(cgi, session_options, response)
        end

        response.finish
      end
    end
  end
end