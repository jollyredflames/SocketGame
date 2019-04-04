#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>

#include "socket.h"
#include "gameplay.h"


#ifndef PORT
    #define PORT 57597
#endif
#define MAX_QUEUE 5

void add_player(struct client **top, int fd, struct in_addr addr);
void announce_turn(struct game_state *game);
void remove_player(struct client **top, int fd);

/*Add Client to game as head of linked list*/
void add_client(Game *game, Client *client){
    client->next = game->head;
    game->head = client;
    if(!game->has_next_turn){
        game->has_next_turn = game->head;

    }
}

/*Remove person with file descriptor fd from new_players list. This does not free/delete Client
 or close Client's fd*/
void remove_player_from_new_players_list(struct client **top, int fd){
    struct client **p;
    int i = 0;
    for (p = top; *p && (*p)->fd != fd; p = &(*p)->next, i++);
    //p now refers to client we want to remove. if it exists
    if (*p) {
        struct client *t = (*p)->next;
        printf("Removing client %d %s from new_players\n", fd, inet_ntoa((*p)->ipaddr));
        *p = t;
        if (i == 0){
            top = p;
        }
    } else {
        fprintf(stderr, "Trying to remove fd %d, but I don't know about it\n",
                fd);
    }
};

/*Return 1 if name is valid else 0. To be valid, name must not be taken*/
int is_valid_name(Game *game, char *name){
    Client *curr = game->head;
    while(curr != NULL){
        if(!strcmp(name, curr->name)){
            return 0;
        }
        curr = curr->next;
    }
    return 1;
}

/* These are some of the function prototypes that we used in our solution
 * You are not required to write functions that match these prototypes, but
 * you may find the helpful when thinking about operations in your program.
 */
/* Send the message in outbuf to all clients */
void broadcast(struct game_state *game, char *outbuf, Client *not_this_client){
    Client *curr = game->head;
    char out[strlen(outbuf)+3];
    strcpy(out, outbuf);
    out[strlen(outbuf)] = '\r';
    out[strlen(outbuf)+1] = '\n';
    out[strlen(outbuf)+2] = '\0';
    while(curr != NULL){
        if(curr == not_this_client){
            curr = curr->next;
            continue;
        }
        int err = write(curr->fd, out, strlen(out));
        if(err == -1){
            //this client has disconnected
            fprintf(stderr, "Client %s has disconnected; broadcast failed\n", inet_ntoa(curr->ipaddr));
            remove_player(&game->head, curr->fd);
        }
        curr = curr->next;
    }
};

/*Announce "It's XXX's turn" where XXX is has_next_turn name. If you are XXX just say 'your'*/
void announce_turn(struct game_state *game){
    if(!game->head){
        return;
    }
    if(!game->has_next_turn){
        if(write((game->head)->fd, "Your guess?\r\n", 13) == -1){
            fprintf(stderr, "Client %s has disconnected; broadcast failed\n", inet_ntoa((game->head)->ipaddr));
            remove_player(&game->head, (game->head)->fd);
        }
        return;
    }
    char longest_possible_str [45];
    sprintf(longest_possible_str, "It's %s\'s turn", (game->has_next_turn)->name);
    broadcast(game, longest_possible_str, game->has_next_turn);
    if(write((game->has_next_turn)->fd, "Your guess?\r\n", 13) == -1){
        fprintf(stderr, "Client %s has disconnected; broadcast failed\n", inet_ntoa((game->has_next_turn)->ipaddr));
        remove_player(&game->head, (game->has_next_turn)->fd);
    }
};

/* Move the has_next_turn pointer to the next active client and broadcast the
 game status message*/
void advance_turn(Game *game){
    char message[MAX_MSG];
    status_message(message, game);
    broadcast(game, message, NULL);
    if((game->has_next_turn)->next == NULL){
        game->has_next_turn = game->head;
        return;
    }
    game->has_next_turn = (game->has_next_turn)->next;
}

/*Game has been won. Announce player who just played 'won' the game and advance turn*/
void announce_winner(struct game_state *game, struct client *winner){
    char longest_possible_str [45];
    sprintf(longest_possible_str, "Player %s won", (game->has_next_turn)->name);
    advance_turn(game);
    broadcast(game, longest_possible_str, NULL);
};

/*Announce dropped connection by player p*/
void announce_dropped_connection(Game *game, char *name){
    char longest_possible_str [70];
    sprintf(longest_possible_str, "Player %s has left the lobby. Bye Bye!", name);
    broadcast(game, longest_possible_str, NULL);
};

/*Game has been lost. Announce word and advance turn*/
void announce_loss(struct game_state *game){
    char longest_possible_str [100];
    sprintf(longest_possible_str, "Boo Hoo Game Ended! The word was %s. Starting new game\r\n", game->word);
    advance_turn(game);
    broadcast(game, longest_possible_str, NULL);
};

/* The set of socket descriptors for select to monitor.
 * This is a global variable because we need to remove socket descriptors
 * from allset when a write to a socket fails.
 */
fd_set allset;


/* Add a client to the head of the linked list
 */
void add_player(struct client **top, int fd, struct in_addr addr) {
    struct client *p = malloc(sizeof(struct client));

    if (!p) {
        perror("malloc");
        exit(1);
    }
    
    printf("Adding client %s\n", inet_ntoa(addr));

    p->fd = fd;
    p->ipaddr = addr;
    p->name[0] = '\0';
    p->in_ptr = p->inbuf;
    p->inbuf[0] = '\0';
    p->next = *top;
    *top = p;
}

/*Given the return code from update_guess() [gameplay.c], set message to something helpful
 for guessing client if necessary*/
void set_message_to_guess_ret_code(Game *game, Client *p, int guess_return_code, char *message, char *data){
    switch (guess_return_code) {
        case -1:
            printf("Player %s guessed non a-z character\n", p->name);
            strcat(message, "Please Enter a letter between a-z.\r\n");
            break;
        case 0:
            printf("Player %s guessed character that was already guessed\n", p->name);
            strcat(message, "Sorry! This letter was already guessed. Please try again\r\n");
            break;
        case 1:
            strcat(message, "Wow! You Get to guess again!\r\n");
            break;
        case 2:
            sprintf(message, "%c is not in the word\r\n", data[0]);
            advance_turn(game);
            break;
    }
}

/* Removes client from the linked list and closes its socket.
 * Also removes socket descriptor from allset 
 */
void remove_player(struct client **top, int fd) {
    struct client **p;
    int i = 0;
    for (p = top; *p && (*p)->fd != fd; p = &(*p)->next, i++);
    //p now refers to client we want to remove. if it exists
    
    if (*p) {
        struct client *t = (*p)->next;
        printf("Removing client %d %s\n", fd, inet_ntoa((*p)->ipaddr));
        FD_CLR((*p)->fd, &allset);
        close((*p)->fd);
        free(*p);
        *p = t;
        if (i == 0){
            top = p;
        }
    } else {
        fprintf(stderr, "Trying to remove fd %d, but I don't know about it\n",
                 fd);
    }
}


int main(int argc, char **argv) {
    int clientfd, maxfd, nready;
    struct client *p;
    struct sockaddr_in q;
    fd_set rset;
    
    if(argc != 2){
        fprintf(stderr,"Usage: %s <dictionary filename>\n", argv[0]);
        exit(1);
    }
    
    // Create and initialize the game state
    struct game_state game;

    srandom((unsigned int)time(NULL));
    // Set up the file pointer outside of init_game because we want to 
    // just rewind the file when we need to pick a new word
    game.dict.fp = NULL;
    game.dict.size = get_file_length(argv[1]);

    init_game(&game, argv[1]);
    
    // head and has_next_turn also don't change when a subsequent game is
    // started so we initialize them here.
    game.head = NULL;
    game.has_next_turn = NULL;
    
    /* A list of client who have not yet entered their name.  This list is
     * kept separate from the list of active players in the game, because
     * until the new playrs have entered a name, they should not have a turn
     * or receive broadcast messages. In other words, they can't play until
     * they have a name.
     */
    struct client *new_players = NULL;
    
    struct sockaddr_in *server = init_server_addr(PORT);
    int listenfd = set_up_server_socket(server, MAX_QUEUE);
    
    // initialize allset and add listenfd to the
    // set of file descriptors passed into select
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
    // maxfd identifies how far into the set to search
    maxfd = listenfd;
    while (1) {
        
        announce_turn(&game);
        
        // make a copy of the set before we pass it into select
        rset = allset;
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if (nready == -1) {
            perror("select");
            continue;
        }

        if (FD_ISSET(listenfd, &rset)){
            printf("A new client is connecting\n");
            clientfd = accept_connection(listenfd);

            FD_SET(clientfd, &allset);
            if (clientfd > maxfd) {
                maxfd = clientfd;
            }
            printf("Connection from %s\n", inet_ntoa(q.sin_addr));
            add_player(&new_players, clientfd, q.sin_addr);
            char *greeting = WELCOME_MSG;
            if(write(clientfd, greeting, strlen(greeting)) == -1) {
                fprintf(stderr, "Write to client %s failed\n", inet_ntoa(q.sin_addr));
                remove_player(&new_players, p->fd);
            };
        }
        
        /* Check which other socket descriptors have something ready to read.
         * The reason we iterate over the rset descriptors at the top level and
         * search through the two lists of clients each time is that it is
         * possible that a client will be removed in the middle of one of the
         * operations. This is also why we call break after handling the input.
         * If a client has been removed the loop variables may not longer be 
         * valid.
         */
        int cur_fd;
        for(cur_fd = 0; cur_fd <= maxfd; cur_fd++) {
            if(FD_ISSET(cur_fd, &rset)) {
                
                // Check if this socket descriptor is an active player
                for(p = game.head; p != NULL; p = p->next) {
                    if(cur_fd == p->fd) {
                        char buf[MAX_MSG] = "";
                        char data[4] = "";
                        int read_till_now = 0;
                        int finished_read = 0;  //1 when full line has been read
                        
                        while(!finished_read){
                            int length_of_partial = read(cur_fd, &buf, MAX_MSG);
                            printf("[%d] Read %d bytes\n", cur_fd, length_of_partial);
                            if(length_of_partial == 0){
                                //Happens if client sends control + C when turn to guess
                                finished_read = 1;
                                data[0] = '\0';
                            }
                            if(buf[length_of_partial-1] == '\n'){
                                printf("[%d] Found newline character\n", cur_fd);
                                buf[length_of_partial-2] = '\0';
                                buf[length_of_partial-1] = '\0';
                                finished_read = 1;
                            }
                            read_till_now += length_of_partial;
                            
                            if(read_till_now < 4){
                                strcat(data, buf);
                            }
                        }
                        
                        if(game.has_next_turn->fd == cur_fd){
                            //It was this client's turn
                            int guess_return_code;
                            
                            if(strlen(data) == 1){
                                guess_return_code = update_guess(data[0], &game);
                            }else{
                                //Client guess contained too many characters
                                if(write(cur_fd, "Please enter a single letter!\r\n", 31) == -1){
                                    fprintf(stderr, "Client %s disconnected after providing an incorrectly formatted string\n", inet_ntoa(p->ipaddr));
                                    announce_dropped_connection(&game, p->name);
                                    advance_turn(&game);
                                    remove_player(&game.head, p->fd);
                                }
                                break;
                            }
                            
                            char message[100] = "";
                            set_message_to_guess_ret_code(&game, p, guess_return_code, message, data);
                            
                            if(guess_return_code > 0){
                                char broadcast_guess [50];
                                printf("Player %s guessed %c\n", p->name, data[0]);
                                sprintf(broadcast_guess, "%s guesses: %c", (game.has_next_turn)->name, data[0]);
                                broadcast(&game, broadcast_guess, p);
                            }
                            
                            if(guess_return_code == 1){
                                char message[MAX_MSG];
                                status_message(message, &game);
                                broadcast(&game, message, NULL);
                            }
                            
                            if(guess_return_code == 3){
                                announce_winner(&game, p);
                                init_game(&game, argv[1]);
                                break;
                            }
                            
                            if(guess_return_code == 4){
                                announce_loss(&game);
                                init_game(&game, argv[1]);
                                break;
                            }
                            
                            if(guess_return_code < 3){
                                if(write(cur_fd, message, strlen(message)) == -1){
                                    fprintf(stderr, "Client %s disconnected after writing a guess\n", inet_ntoa(p->ipaddr));
                                    announce_dropped_connection(&game, p->name);
                                    if(guess_return_code != 2){
                                        advance_turn(&game);
                                    }
                                    remove_player(&game.head, p->fd);
                                }
                                break;
                            }
                            
                            
                        }else{
                            //It was not this client's turn
                            printf("Player %s tried guessing out of turn\n", p->name);
                            if(write(cur_fd, "It is not your turn to guess\r\n", 30) == -1){
                                fprintf(stderr, "Client %s disconnected after trying to guess out of phase\n", inet_ntoa(p->ipaddr));
                                announce_dropped_connection(&game, p->name);
                                remove_player(&game.head, p->fd);
                            }
                        }
                        
                        break;
                    }
                }
        
                // Check if any new players are entering their names
                for(p = new_players; p != NULL; p = p->next) {
                    if(cur_fd == p->fd) {
                        char buf[MAX_MSG] = "";
                        char name[30] = "";
                        int successful = 0;
                        int read_till_now = 0;
                        int finished_read = 0;  //1 when full message recieved from client
                        while(!finished_read){
                            //do buffered read
                            int length_of_partial = read(cur_fd, &buf, MAX_MSG);
                            if(buf[length_of_partial-1] == '\n'){
                                buf[length_of_partial-2] = '\0';
                                buf[length_of_partial-1] = '\0';
                                finished_read = 1;
                            }
                            read_till_now += length_of_partial;
                            
                            if(read_till_now < 29){
                                //We only have to be mindful of input of provided input meets 29 character specification
                                //29 characters as client->name is stored in 30 byte string with null terminator
                                strcat(name, buf);
                                successful = 1;
                            }
                            
                            if(read_till_now > 29 && finished_read){
                                successful = 0;
                                if(write(cur_fd, "Names Cannot Be More Than 29 characters!\r\n", 42) == -1){
                                    fprintf(stderr, "Client %s disconnected after providing a name that was too long\n", inet_ntoa(p->ipaddr));
                                    remove_player(&new_players, p->fd);
                                }
                            }
                        }
                        if(successful){
                            //Name is 29 characters or less
                            if(is_valid_name(&game, name)){
                                //Name is unique to this game session
                                remove_player_from_new_players_list(&new_players, p->fd);
                                strcpy(p->name, name);
                                add_client(&game, p);
                                char broadcast_joiner [55];
                                sprintf(broadcast_joiner, "%s has joined the game", name);
                                broadcast(&game, broadcast_joiner, NULL);
                                char message[MAX_MSG];
                                status_message(message, &game);
                                if(write(cur_fd, message, strlen(message)) == -1){
                                    fprintf(stderr, "Client %s disconnected after providing a valid name and joining the game\n", inet_ntoa(p->ipaddr));
                                    remove_player(&new_players, p->fd);
                                }
                            }else{
                                if(write(cur_fd, "Name already used! Try another one\r\n", 36) == -1){
                                    fprintf(stderr, "Client %s disconnected after providing a name that was already present\n", inet_ntoa(p->ipaddr));
                                    remove_player(&new_players, p->fd);
                                }
                            }
                        }
                        
                        break;
                    } 
                }
            }
        }
    }
    return 0;
}


