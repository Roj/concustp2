# concustp2
Repositorio del TP2 de concus


**Proposiciones:**

* Tener un proceso para cada "microservicio" (clima y monedas), y otro proceso que sea el servidor (el "portal") que le hace requests a esos dos. Después, que cada cliente y el administrador sean también procesos.
* Inventar primero que nada una abstracción tipo TDA de los sockets, con primitivas para poder usarlos directamente. 
* Inventar alguna suerte de struct que represente los "mensajes" que se envian todos estos procesos, para poder establecer así una suerte de protocolo y que se envíen siempre las mismas cosas. Una vez que llegásemos a tener dos procesos comunicándose con estos mensajitos y los sockets abstractos, el resto debería ser más o menos fácil.

