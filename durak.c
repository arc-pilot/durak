/*
Durak card game implementation
https://en.wikipedia.org/wiki/Durak
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct Card {
  int value;
  int suit;
};

struct Stack {
  int length;
  struct Card** card;
};

struct Player {
  struct Stack pack;
  struct Stack* (*play)(struct Stack*, struct Stack*);
  char* name;
};

int get_adv(void);
int set_adv(struct Stack*);

int get_value_offset(void);

struct Stack* get_cards(int);

void push_card(struct Player*, struct Stack*);
void pop_card(struct Player*, struct Stack*);

void shuffle(struct Card**, int);

struct Stack* comp(struct Stack*, struct Stack*);
struct Stack* human(struct Stack*, struct Stack*);

struct Card* deck_init(int);
void deck_clear(struct Card*);

struct Stack* stack_init(struct Card*, int);
void stack_clear(struct Stack*);

struct Player* players_init(int, int, struct Stack* (*[])(struct Stack*, struct Stack*), char**);
void players_clear(struct Player*, int);

struct Player** turn_init(struct Player*, int);
void turn_clear(struct Player**);

struct Stack* turn(struct Player*, struct Stack*);
void print_stack(struct Player*, struct Stack*);

struct Stack* base_stack;

int deck_len, value_offset, adv_suit, players_num;

const char* ranks[] = {"2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A"};
const char suit[] = {'C', 'S', 'D', 'H'};

int get_adv(void) {
  extern int adv_suit;
  return adv_suit;
}

int set_adv(struct Stack* base) {
  return (base->card[base->length - 1])->suit;
}

int get_value_offset(void) {
  extern int value_offset;
  return value_offset;
}

struct Stack* get_cards(int size) {
  extern struct Stack* base_stack;
  if(base_stack->length == 0)
    return NULL;
  if(size > base_stack->length)
    size = base_stack->length;
  struct Stack* slice = (struct Stack*) malloc(sizeof(struct Stack));
  slice->card = (struct Card**) malloc(size * sizeof(struct Card*));
  slice->length = size;
  extern int deck_len;
  memcpy(slice->card, base_stack->card + (deck_len - base_stack->length), size * sizeof(struct Card*));
  base_stack->length -= size;
  return slice;
}

void push_card(struct Player* player, struct Stack* stk) {
  for(int i = 0; i < stk->length; i++)
    player->pack.card[player->pack.length + i] = stk->card[i];
  player->pack.length += stk->length;
}

void pop_card(struct Player* player, struct Stack* stk) {
  for(int x = 0; x < stk->length; x++) {
    for(int y = 0; y < player->pack.length; y++) {
      if(player->pack.card[y] == stk->card[x]) {
        if(y != player->pack.length - 1) {
          for(int z = y; z < player->pack.length; z++)
            player->pack.card[z] = player->pack.card[z + 1];
        }
        --player->pack.length;
      }
    }
  }
}

void shuffle(struct Card** array, int len) {
  srand(time(NULL));
  for(int i = 0; i < len; i++) {
    int rnd = rand() % len;
    struct Card* tmp = array[i];
    array[i] = array[rnd];
    array[rnd] = tmp;
  }
}

struct Stack* comp(struct Stack* pl, struct Stack* enemy) {
  const int adv = get_adv();
  static struct Card* sorted[52];

  memcpy(sorted, pl->card, pl->length * sizeof(struct Card *));

  {
    for(int b = pl->length - 1; b; b--)
      for(int a = 0; a < b; a++) {
        int sort_suit = sorted[a]-> suit;
        int sort_value = sorted[a]->value;
        int sort_next_suit = sorted[a + 1]->suit;
        int sort_next_value = sorted[a + 1]->value;
        if((((sort_suit != adv && sort_next_suit != adv) || (sort_suit == adv && sort_next_suit == adv)) && (sort_value > sort_next_value)) || (sort_suit == adv && sort_next_suit != adv)) {
          struct Card* tmp = sorted[a];
          sorted[a] = sorted[a + 1];
          sorted[a + 1] = tmp;
        }
      }
  }

  struct Stack* answer = (struct Stack*) malloc(sizeof(struct Stack));
  answer->length = 0;

  if(enemy != NULL) {
    for(int i = 0; i < pl->length; i++) {
      int sorted_suit = sorted[i]->suit;
      int sorted_value = sorted[i]->value;
      int enemy_suit = (*(enemy->card))->suit;
      int enemy_value = (*(enemy->card))->value;
      if((sorted_suit == enemy_suit && sorted_value > enemy_value) || (sorted_suit == adv && enemy_suit != adv)) {
        if(i + enemy->length <= pl->length) {
          answer->length = enemy->length;
          answer->card = (struct Card**) malloc(answer->length * sizeof(struct Card*));
          memcpy(answer->card, sorted + i, answer->length * sizeof(struct Card*));
          break;
        }
      }
    }
    if(answer->length == 0) {
      free(answer);
      answer = NULL;
    }
  }
  else {
    answer->length = 1;
    answer->card = (struct Card**) malloc(answer->length * sizeof(struct Card*));
    answer->card[0] = sorted[0];
  }
  return answer;
}

struct Stack* human(struct Stack* pl, struct Stack* enemy) {
  extern const char* ranks[];
  extern const char suit[];

  int adv = get_adv();
  puts("Your cards are:");
  for(int i = 0; i < pl->length; i++) {
    struct Card* temp = pl->card[i];
    if(temp != NULL)
      printf("%d : %s %c\n", i, ranks[temp->value + get_value_offset()], suit[temp->suit]);
  }

  int idx;
  do {
    puts("Enter index of your stack:");
    scanf("%d", &idx);
  } while(idx < -1 || idx > pl->length - 1);

  if(enemy != NULL) {
    if(idx == -1) {
      return NULL;
    }
    else {
      struct Card* my = pl->card[idx];
      struct Card* en = enemy->card[0];
      if(en->suit == adv) {
        if((my->suit != adv) || (my->value < en->value))
          return NULL;
      }
      else {
        if(my->suit != adv) {
          if((my->suit != en->suit) || (my->value < en->value))
            return NULL;
        }
      }
    }
  }

  struct Stack* answer = (struct Stack*) malloc(sizeof(struct Stack));
  answer->card = (struct Card**) malloc(sizeof(struct Card*));
  answer->card[0] = pl->card[idx];
  answer->length = 1;
  return answer;
}

struct Card* deck_init(int len) {
  struct Card* deck = (struct Card*) malloc(len * sizeof(struct Card));
  for(int i = 0; i < len; i++) {
    deck[i].value = i / 4;
    deck[i].suit = i % 4;
  }
  return deck;
}

void deck_clear(struct Card* deck) {
  free(deck);
}

struct Stack* stack_init(struct Card* deck, int len) {
  struct Stack* stk = (struct Stack*) malloc(sizeof(struct Stack));
  stk->card = (struct Card**) malloc(len * sizeof(struct Card*));
  stk->length = len;
  for(int i = 0; i < len; i++)
    stk->card[i] = deck + i;
  return stk;
}

void stack_clear(struct Stack* deck) {
  free(deck->card);
  free(deck);
}

struct Player* players_init(int num, int len, struct Stack* (*ai[])(struct Stack*, struct Stack*), char** names) {
  struct Player* players = (struct Player*) malloc(num * sizeof(struct Player));
  for(int i = 0; i < num; i++, ai++) {
    players[i].pack.card = (struct Card**) calloc(len, sizeof(struct Card*));
    players[i].pack.length = 0;
    players[i].name = *names++;
    players[i].play = *ai;
    struct Stack* stk = get_cards(6);
    push_card(players + i, stk);
    stack_clear(stk);
  }
  return players;
}

void players_clear(struct Player* players, int num) {
  for(int i = 0; i < num; i++) {
    free(players[i].pack.card);
  }
  free(players);
}


struct Player** turn_init(struct Player* pls, int pl_num) {
  struct Player** turn = (struct Player**) malloc(pl_num * sizeof(struct Player*));
  struct {
    int value, index;
  } max;
  max.value = -1;
  max.index = -1;
  int adv = get_adv();
  for(int i = 0; i < pl_num; i++) {
    for(int k = 0; k < 6; k++) {
      struct Card* current = pls[i].pack.card[k];
      if((current->suit == adv) && (current->value > max.value)) {
        max.value = current->value;
        max.index = i;
      }
    }
    turn[i] = pls + i;
  }

  if(max.index == -1) {
    srand(time(NULL));
    max.index = rand() % pl_num;
  }

  if(max.index) {
    struct Player* temp = turn[0];
    turn[0] = turn[max.index];
    turn[max.index] = temp;
  }

  return turn;
}

void turn_clear(struct Player** turn) {
  free(turn);
}

struct Stack* turn(struct Player* pl, struct Stack* enemy) {
  return pl->play(&pl->pack, enemy);
}

void print_stack(struct Player* player, struct Stack* table) {
  if(table == NULL) {
    printf("Player:%s | Fail\n", player->name);
  } else {
    extern const char* ranks[];
    extern const char suit[];
    for(int i = 0; i < table->length; i++) {
      printf("Player:%s | %s %c\n", player->name, ranks[(*(table->card))->value + get_value_offset()], suit[(*(table->card))->suit]);
    }
  }
}

int main (int argc, char* argv[]) {
  if(argc == 3 && ((players_num = atoi(argv[1])) >= 2 && atoi(argv[1]) <= 4)
  && ((deck_len = atoi(argv[2])) == 1 || atoi(argv[2]) == 0)) {
    if(deck_len == 0) {
      deck_len = 36;
      value_offset = 4;
    }
    else if (deck_len == 1) {
      deck_len = 52;
      value_offset = 0;
    }
  }
  else {
    fprintf(stderr, "Enter number of players (2-4) and deck size (0 = 36, 1 = 52)\n");
    exit(1);
  }

  struct Stack* (*ai[])(struct Stack*, struct Stack*) = {human, comp, comp, comp};

  char* names[] = {"Human", "Comp1", "Comp2", "Comp3", "Comp4"};

  struct Card* base_deck = deck_init(deck_len);
  base_stack = stack_init(base_deck, deck_len);
  shuffle(base_stack->card, deck_len);
  adv_suit = set_adv(base_stack);
  struct Player* players = players_init(players_num, deck_len, ai, names);
  struct Player** turn_queue = turn_init(players, players_num);

  printf("Adv suit is: %c\n", suit[get_adv()]);
  struct Stack* table = NULL;
  int active_players = players_num;
  int i = 0;
  while(active_players > 0) {
    while(turn_queue[i] == NULL) {
      i++;
      i = i % players_num;
    }
    struct Stack* temp = table;
    struct Stack* slice;

    table = turn(turn_queue[i], table);
    if(temp == NULL) {
      if(table != NULL) {
        print_stack(turn_queue[i], table);
        pop_card(turn_queue[i], table);
        if(turn_queue[i]->pack.length < 6) {
          slice = get_cards(table->length);
        } else {
          slice = NULL;
        }
        if(slice != NULL) {
          push_card(turn_queue[i], slice);
          stack_clear(slice);
        }
        else {
          if(turn_queue[i]->pack.length == 0) {
            turn_queue[i] = NULL;
            active_players--;
          }
        }
        i++;
        i = i % players_num;
      }
      else {
        printf("Error\n");
        break;
      }
    }
    else {
      if(table != NULL) {
        print_stack(turn_queue[i], table);
        pop_card(turn_queue[i], table);
        if(turn_queue[i]->pack.length < 6) {
          slice = get_cards(table->length);
        } else {
          slice = NULL;
        }
        if(slice != NULL) {
          push_card(turn_queue[i], slice);
          stack_clear(slice);
        } else {
          if(turn_queue[i]->pack.length == 0) {
            turn_queue[i] = NULL;
            active_players--;
          }
        }
        stack_clear(table);
        stack_clear(temp);
        table = NULL;
      }
      else {
        print_stack(turn_queue[i], table);
        push_card(turn_queue[i], temp);
        stack_clear(temp);
        i++;
        i = i % players_num;
        if(active_players == 1) {
          break;
        }
      }
    }
  }

  puts("");

  if(active_players == 0) {
    puts("Draw");
  } else {
    int j;
    for(j = 0; j < players_num; j++) {
      if(turn_queue[j] != NULL) {
        printf("Player %s lose\n", turn_queue[j]->name);
      }
    }
  }

  players_clear(players, players_num);
  turn_clear(turn_queue);
  stack_clear(base_stack);
  deck_clear(base_deck);

  return 0;
}
