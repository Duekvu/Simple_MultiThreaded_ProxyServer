# Simple_MultiThreaded_ProxyServer

Multi Threaded ProxyServer that can handle the GET requests from multiple clients, and using HTTP/1.0.   

## Intructions

The executable is called MyProxy that takes as its first argument a port to listen on.

## To test the proxy:
You can configure Firefox to use your proxy server as its web proxy as follows (more recent versions of Firefox could differ slightly. Note that it is your own responsibilities to figure out the configurations):  
1. Go to the 'Edit' menu.  
2. Select 'Preferences'. Select 'Advanced' and then select 'Network'. 
3. Under 'Connection', select 'Settings...'.  
4. Select 'Manual Proxy Configuration'. Enter the hostname and port where your proxy program is running. 
5. Save your changes by selecting 'OK' in the connection tab and then select 'Close' in the preferences tab. 

<b>Because Firefox defaults to using HTTP/1.1 and the proxy speaks HTTP/1.0, there are a couple of minor changes  that  need  to  be  made  to  Firefox's  configuration.  Fortunately,  Firefox  is  smart  enough  to  know  when it is connecting through a proxy, and has a few special configuration keys that can be used to tweak the browser's behavior. </b>
1. Type 'about:config' in the title bar.
2. In the search/filter bar, type 'network.http.proxy'
3. You should see three keys: network.http.proxy.keepalive, network.http.proxy.pipelining, and network.http.proxy.version.
4. Set keepalive to false. Set version to 1.0. Make sure that pipelining is set to false.

To configure Internet Explorer,  you should also do the following to make Internet Explorer work in a HTTP 1.0 compatible mode with your proxy:
1.Under Internet Options, select the 'Advanced' tab.2.Scroll down to HTTP 1.1 Settings. Uncheck 'Use HTTP 1.1 through proxy connections' 


 

