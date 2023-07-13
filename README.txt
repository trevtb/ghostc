##### GHOSTc v0.1 README #####

CONTENT:
    I. About
    II. Install
    III: Configuration
    IV: Usage
_________________________________

- I - About -
---------------------------------

GHOST stands for Great Hope Of Secure Transmission and is an attempt to create a real secure
messaging system with full protection against surveillance.

GHOSTc is the mailer module and user administration tool of the GHOST secure chat application.
It maintains a mySQL database with encrypted user and mail accounts and an API key.

The purpose of this application is to generate a new API key and mail it to all users stored
in the database using the mail accounts provided when the application is being executed,
which is usually once a week.

Mails sent are always tunneled through TOR over a SOCKS5 PROXY on the local host.

The application features SMTP load balancing, splitting the sending process equally on all
available mail accounts and using the maximum allowed recipient count. Other recipients are
always added to the BCC field so mail adresses are not visible to other users.
There also is a delay value for controlling the maximum allowed messages per hour for each account.

The application supports SSL and TLS encryption and it is recommended to use TLS.
All information in the database are securely encrypted and get decrypted just in time, meaning
they also won't be stored in cleartext within memory when the application is running.
Only encrypted hashes of the user passwords are being stored, same goes for the randomly
created api key. Hashes are SHA1, encryption is AES-256.

GhostC provides command access to listing, adding, deleting and editing user and mail accounts.
It also has an installation feature which automatically creates the needed tables for operation.

_________________________________

- II - Installation -
---------------------------------
The following commands have to be run with root privileges.
Use 'dpkg -i ghostc_0.1-1_amd64.deb' for installation.
If it complains about missing dependencies run 'apt-get -f install'.
This will install missing dependencies. Afterwards run 'dpkg -i ghostc_0.1-1_amd64.deb' again.

The installation routine will create a ghost group and a user under whiche it will run.
Only users of the ghost group will have access to the program files and the installed cronjob
will run under the provided user.

GHOSTc uses the following files:
    /usr/bin/ghostc         // The executable only executable by root and ghost group.
    /etc/ghost/ghost.cfg    // The config file, only readable by root and ghost group.
    /etc/ghost/key.key      // The encryption key only readable by root and ghost group.
    /etc/cron.d/ghostc      // The cronjob for regular execution (default: Sunday, 23:59)

GHOSTc needs a working mySQL server with a valid database and a user with read and write access to it.
You also need a working TOR client providing a SOCKS5 PROXY which GHOSTc can use.

After setting up TOR and your mySQL server you must create a database with the name you set or will set
in ghost.cfg.
It is also recommended to create a dedicated user for access to this database and not
use the root account with GHOSTc for security reasons.
Also check your TOR config and make sure TOR provides PROXY support and the port matches the
provided or if not provided default one.

After you have mySQL and TOR set up, run 'ghostc --install'. This will create all required
tables in the database.

The installed cron script calls 'ghostc --recrypt' and 'ghostc --mail' once a week (see IV-Usage).

Read 'Configuration' for detailed configuration instructions.

_________________________________

- III - Configuration -
---------------------------------

GHOSTc is being configured in /etc/ghost/ghost.cfg. There are several values you can adjust:
    -> db_host: The hostname or IP address of the mySQL database server.
    -> db_port: The port of the mySQL server (optional).
    -> db_name: The database name which GHOSTc will use. It must provide read and write access.
    -> db_user: The database user GHOSTc will use to connect to the database.
    -> db_pass: The password for the given user.
    -> tor_port: The local port on which the installed TOR client is listening for icoming PROXY connections (optional).
The db_port and tor_port parameters are optional, all other parameters must be set.
By default db_port 3306 and tor_port 9050 are being used.

Before running the application for the first time and after you have ensured that the mySQL server
is correctly set up and the database and user information are correct,
you can issue 'ghostc --install' to create all needed tables for operation.
This command does not create tables if they already exist, meaning it won't override an
existing installation.

GHOSTc automatically takes care of the encryption key and stores it in the file /etc/ghost/key.key.
If you call 'ghostc --recrypt' a new encryption key will be generated and all data in the database will be reencrypted.

The execution is handled by the cron script /etc/cron.d/ghostc which calls --recrypt and --mail once a week.

_________________________________

- IV - Usage
---------------------------------

The executable /usr/bin/ghostc accepts the following command line arguments:

    --help: Display a command overview.
    --install: Creates the database and tables. Database name will be taken from the config file.
    --recrypt: Re-encrypts all database content with a new random key.
    --auth --user <user> --pass <password_hash>: Check if user data is valid. Returns the request timeout in seconds or false.
    --list-apikey: Lists the current api key stored in the database.
    --list-users: Lists all users stored in the database.
    --list-accounts: Lists all mail accounts stored in the database.
    --print-reqpwd --id <id>: Prints the request password for the given request id in cleartext.
    --list-openreqs --user <user>: Lists the open chat requests for the given username.
    --delreq --id <id>: Deletes the request with the given id.
    --mail: Generates a new API key and mails it to all users using automatic SMTP load balancing.
    --initiate --user <user> --recp <recipient>: Initiates a new chat request by notify the user and writing the request to the database.
    --adduser --mail <email> --user <username> --pass <password>: Adds a new user to the database.
    --deluser <id>: Delete the user corresponding to the given id.
    --updateuser --id <id> --attribute <attribute> --value <value>: Update a user's attribute.
        Example: ghostc --updateuser --id 1 --attribute username --value newname
    --changesettings --user <username> --mail <new_mail> --pass <new_pass>: Quick way to update the user credentials. You MUST supply 'empty' for empty values.
    --addaccount --host <host> --port <port> --auth <auth> --encryption <ssl_tls> --user <username>
                --pass <password> --mail <email> --max_recps <max_recipients> --delay <delay>
        auth: Authentication required. Yes=1, No=0.
        tssl_tls: Encryption. None=0, SSL=1, TLS=2.
        [optional] max_recipients: Maximum recipients per mail (default: 10).
        [optional] delay: Time in milliseconds to wait between each mail (default: 5000).
    --delaccount <id>: Delete the account corresponding to the given id.
    --updateaccount --id <id> --attribute <attribute> --value <value>: Update an account attribute.
        Example: ghostc --updateaccount --id 1 --attribute email --value you@domain.com
    --enterchat --user <user> --reqid <reqid>: Lets the user enter the chat with the given request.
    --addchatmsg --reqid <id> --user <username> --message <text>: Adds a new chat message to the server. After completion this function prints all the chat messages for the request.
        ATTENTION: The text needs to be URL encoded.
    --getchatmessages --reqid <id>: Get the chat messages for the given request.
    --cleantimeouts: Deletes all timed out requests and their corresponding chat messages.

By default '--recrypt' and '--mail' should be called (exactly in this order) in a given time intervall
like once a week. This is being handled by the cron script /etc/cron.d/ghostc.
When issuing the mail command which needs a TOR connection, two requests to http://ip-api.com are being
made to retrieve the external IP address before and after using the installed TOR client.
This ensures that TOR is being used so that the real IP is properly masqueraded when sending mails.
