#include <pony/pony.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct counter_t
{
  uint64_t count;
} counter_t;

enum
{
  MSG_INIT,
  MSG_INC,
  MSG_GETANDRESET,
  MSG_RESPONSE
};

static pony_msg_t m_init = {0, {{NULL, 0, PONY_PRIMITIVE}}};
static pony_msg_t m_inc = {0, {{NULL, 0, PONY_PRIMITIVE}}};
static pony_msg_t m_getandreset = {1, {{NULL, 0, PONY_ACTOR}}};
static pony_msg_t m_response = {1, {{NULL, 8, PONY_PRIMITIVE}}};

static void trace(void* p)
{
  counter_t* d = p;
  pony_trace64(&d->count);
}

static pony_msg_t* message_type(uint64_t id)
{
  switch(id)
  {
    case MSG_INIT: return &m_init;
    case MSG_INC: return &m_inc;
    case MSG_GETANDRESET: return &m_getandreset;
    case MSG_RESPONSE: return &m_response;
  }

  return NULL;
}

static void dispatch(pony_actor_t* this, void* p, uint64_t id, int argc, pony_arg_t* argv);

static pony_actor_type_t type =
{
  1,
  {trace, sizeof(counter_t), PONY_ACTOR},
  message_type,
  dispatch
};

static void dispatch(pony_actor_t* this, void* p, uint64_t id, int argc, pony_arg_t* argv)
{
  counter_t* d = p;

  switch(id)
  {
    case PONY_MAIN:
    {
      int margc = argv[0].i;
      char** margv = argv[1].p;
      uint64_t count = 0;

      if(margc >= 2)
      {
        count = atoi(margv[1]);
      }

      pony_actor_t* actor = pony_create(&type);
      pony_send(actor, MSG_INIT);

      for(uint64_t i = 0; i < count; i++)
      {
        pony_send(actor, MSG_INC);
      }

      pony_sendp(actor, MSG_GETANDRESET, this);
      break;
    }

    case MSG_INIT:
      d = pony_alloc(sizeof(counter_t));
      d->count = 0;
      pony_set(d);
      break;

    case MSG_INC:
      d->count++;
      break;

    case MSG_GETANDRESET:
      pony_sendi(argv[0].p, MSG_RESPONSE, d->count);
      d->count = 0;
      break;

    case MSG_RESPONSE:
      printf("%lu\n", argv[0].i);
      break;
  }
}

int main(int argc, char** argv)
{
  return pony_start(argc, argv, pony_create(&type));
}