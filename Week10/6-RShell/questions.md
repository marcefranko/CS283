1. How does the remote client determine when a command's output is fully received from the server, and what techniques can be used to handle partial reads or ensure complete message transmission?

_answer here_: The client looks for an EOF characters at the end of a signal it will not stop looking for this until it finds it and only then with the client determine it has fully received it from the server. Techniques that could be utilized to handle partial reads or ensure complete message transmision would be to use command for length or similar patterns.

2. This week's lecture on TCP explains that it is a reliable stream protocol rather than a message-oriented one. Since TCP does not preserve message boundaries, how should a networked shell protocol define and detect the beginning and end of a command sent over a TCP connection? What challenges arise if this is not handled correctly?

_answer here_: TCP is just a stream of bytes. The shell would have to define its own boundaries and when commands are sent take them as strings with respsonses ending wth the EOF character we previously discussed about before. Without similar strucutre, it would have incomplete commands, mixed repones, and just overflows.

3. Describe the general differences between stateful and stateless protocols.

_answer here_: Stateles prototocls dont retain any client info, as each of the request already contains what is needed. Not as efficient. Stateful protocol, on the other hand, mtaintaain information between request and preserves user directory.

4. Our lecture this week stated that UDP is "unreliable". If that is the case, why would we ever use it?

_answer here_: UDP is utilized when we need speed rather than reliability. It eliminates some of the steps and therefore provides a lower latency. Applications running in real time beneft greatly a data is being constntly refreshed, like a stream.

5. What interface/abstraction is provided by the operating system to enable applications to use network communications?

_answer here_: The socket interface is provided by the operating system to enable applications to use network communications. It use file descriptors and let applications receive/send data. They have a complex protocal system which they hide. But the shell needs it to transfer data and establsh connection