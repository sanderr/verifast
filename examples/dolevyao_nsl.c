// Dolev-Yao security verification of the Needham-Schroeder-Lowe public key authentication protocol
// Uses an invariant-based approach inspired by the work of Ernie Cohen and the work of Andrew Gordon et al.

#include "stdlib.h"

/*

Items
=====

Dolev-Yao security of a protocol means that the intended security properties are verified under
the assumption that the cryptographic primitives used, such as key generation, keyed hashes, etc.
are perfect. This assumption is formalized by modelling items sent over public channels not as
bitstrings but as structured values.

For example, we represent the Nth key generated by principal P
as a structured value key_item(P, N). It automatically follows that key_item(P1, N1) == key_item(P2, N2)
if and only if P1 == P2 and N1 == N2. This formalizes the assumption that key generation generates
unique keys.

We represent the HMAC-SHA1 keyed hash of payload item I
generated with the Nth key generated by principal P
as hmacsha1_item(P, N, I). This formalizes the assumption that there are no hash collisions.

API wrapper functions
=====================

We are verifying real, working protocol implementations. These use real cryptographic functions, and
real network I/O functions. The
data objects manipulated by these functions, such as keys, messages, and hashes, are bitstrings.
However, to facilitate associating an item value with
such a physical data object, we wrap these functions in a thin layer that represents the data objects
as objects of type struct item. The wrapper functions are specified in terms of a predicate item(item, i)
that states that item object "item" represents item value "i".

We do not provide an implementation of the wrappers here, but such an implementation is straightforward.

Network I/O and "pub"
=====================

We also declare wrappers for network I/O, called net_send and net_receive. For simplicity, we ignore
addressing aspects: net_send sends an item into the world, and net_receive plucks an arbitrary
item from the world. The world is where the attacker is. We represent the world by a predicate
"world(pub)", where "pub" is a pure function from item values to booleans; that is, it is a predicate
on item values. It specifies an upper bound on which item values are sent into the world. That is,
net_send requires that the item being sent satisfies pub, and net_receive ensures that the item
that is returned satisfies pub.

This file implements two groups of functions: the functions that implement the protocol, and the
function that represents the attacker, called "attacker". Function "attacker" performs all the
operations that an attacker can perform. Specifically, it generates keys and publishes them; it
creates hashes of public items and publishes them; it constructs and destructs pairs; etc.

Crucially, both groups of functions operate on the same world, with the same "pub" function. (This is not
checked by VeriFast.)

Verifying protocol integrity
============================

Protocol integrity means that if the protocol implementation reports to the application that something has
happened, it has indeed happened. For example, in the case of an RPC protocol, if the protocol implementation
on the server reports an incoming request from a given client principal, this client principal must in fact
have made this request. Analogously, if the client protocol reports a response from the server principal, the
application at this principal must indeed have submitted this response.

This is formalized using "event predicates". Here, I use the term "predicate" not in the sense of
a VeriFast predicate, but in the sense of a function that returns a boolean. Specifically, for each event,
an unimplemented fixpoint function is declared that takes as arguments the values that identify the event,
and returns bool.

The example protocol performs RPC between pairs of clients and servers that have agreed on a secret key.
The key agreement mechanism is not modeled; it is assumed that client and server share a secret key.
Specifically, we declare a function shared_with(P, N) that takes a principal P and an index N and that
returns the principal with whom the Nth key created by P is shared, or -1 if the key was not shared. We
assume the client generated the key and shared it with the server.

Note: we do model bad principals. That is, we declare a function bad(P) that returns true if P is bad and
false if it is not. The integrity of the protocol is conditional on the client and server not being bad.
That is, integrity means that "good" principals can use the protocol safely, even if other principals that are
using the protocol are bad. Badness of principal P in this example means that P publishes keys that it creates.
Conversely, if P is not bad, it does not publish keys that it creates.

The example protocol uses two event predicates: "request(C, S, R)" states that client C sent a request item R to
server S; "response(C, S, R, R1)" states that server S responded to request item R from client C with response item
R1.

Defining "pub"
==============

The core task of verifying a protocol implementation is defining "pub". "pub" must be sufficiently weak so that
sends by the protocol functions and sends by the attacker are allowed; but sufficiently strong so that when the
protocol receives an item and the item is valid, the appropriate event predicate follows from it.

*/

// 1. A -> B. {A,NA}_K(B)
// 2. B -> A. {B,NA,NB}_K(A)
// 3. A -> B. {NB}_K(B)

// Goal: NA and NB shared between A and B

struct item;

/*@

inductive item =
  | key_item(int creator, int id, bool isPublicKey, int info)
  | data_item(int id)
  | encrypted_item(int keyCreator, int id, int info, item payload)
  | pair_item(item first, item second);

predicate item(struct item *item, item i);

predicate key(struct item *item, int creator, int id, bool isPublicKey, int info) =
    item(item, key_item(creator, id, isPublicKey, info));

predicate world(fixpoint(item, bool) pub);

predicate principal(int id, int keyCount);
  // Keeps track of the number of keys generated by principal "id"

predicate principals(int count);
  // Keeps track of the number of principals generated, so as to ensure unique principal ids.

lemma void create_principal();
    requires principals(?count);
    ensures principals(count + 1) &*& principal(count, 0);

@*/

struct keypair;

//@ predicate keypair(struct keypair *keypair, int creator, int id, int info);
//@ predicate keypair_request(int info) = emp;

struct keypair *create_keypair();
    //@ requires keypair_request(?info) &*& principal(?principal, ?keyCount);
    //@ ensures principal(principal, keyCount + 1) &*& keypair(result, principal, keyCount, info);

struct item *keypair_get_private_key(struct keypair *keypair);
    //@ requires keypair(keypair, ?creator, ?id, ?info);
    //@ ensures keypair(keypair, creator, id, info) &*& key(result, creator, id, false, info);

struct item *keypair_get_public_key(struct keypair *keypair);
    //@ requires keypair(keypair, ?creator, ?id, ?info);
    //@ ensures key(result, creator, id, true, info);

// check_is_key aborts if the item is not a key.
void check_is_key(struct item *item);
    //@ requires item(item, ?i);
    /*@
    ensures
        switch (i) {
            case key_item(creator, id, isPublicKey, info): return key(item, creator, id, isPublicKey, info);
            case data_item(d): return false;
            case encrypted_item(creator, id, info, payload): return false;
            case pair_item(f, s): return false;
        };
    @*/

struct item *create_data_item(int data);
    //@ requires true;
    //@ ensures item(result, data_item(data));

// item_get_data aborts if the item is not a data item.
int item_get_data(struct item *item);
    //@ requires item(item, ?i);
    /*@
    ensures
        switch (i) {
            case data_item(d): return item(item, i) &*& result == d;
            case key_item(creator, id, isPublicKey, info): return false;
            case encrypted_item(creator, id, info, payload): return false;
            case pair_item(f, s): return false;
        };
    @*/

// This function aborts if the key is a private key.
struct item *encrypt(struct item *key, struct item *payload);
    //@ requires key(key, ?creator, ?id, ?isPublicKey, ?info) &*& item(payload, ?p);
    //@ ensures key(key, creator, id, isPublicKey, info) &*& item(payload, p) &*& item(result, encrypted_item(creator, id, info, p)) &*& isPublicKey;

// This function aborts if the key is a public key.
struct item *decrypt(struct item *key, struct item *item);
    //@ requires key(key, ?creator, ?id, ?isPublicKey, ?info) &*& item(item, ?i);
    /*@
    ensures
        key(key, creator, id, isPublicKey, info) &*& item(item, i) &*& !isPublicKey &*&
        switch (i) {
            case encrypted_item(creator0, id0, info0, p): return creator0 == creator &*& id0 == id &*& info0 == info &*& item(result, p);
            case key_item(creator0, id0, isPublicKey0, info0): return false;
            case data_item(d): return false;
            case pair_item(f, s): return false;
        };
    @*/

// A real implementation must encode the pair such that the first and second components can be extracted correctly.
struct item *create_pair(struct item *first, struct item *second);
    //@ requires item(first, ?f) &*& item(second, ?s);
    //@ ensures item(first, f) &*& item(second, s) &*& item(result, pair_item(f, s));

void net_send(struct item *datagram);
    //@ requires world(?pub) &*& item(datagram, ?d) &*& pub(d) == true;
    //@ ensures world(pub) &*& item(datagram, d);

struct item *net_receive();
    //@ requires world(?pub);
    //@ ensures world(pub) &*& item(result, ?d) &*& pub(d) == true;

struct item *pair_get_first(struct item *pair);
    //@ requires item(pair, ?p);
    /*@
    ensures
        item(pair, p) &*&
        switch (p) {
            case pair_item(f, s): return item(result, f);
            case key_item(creator, id, isPublicKey, info): return false;
            case data_item(d): return false;
            case encrypted_item(creator, id, info, payload): return false;
        };
    @*/

struct item *pair_get_second(struct item *pair);
    //@ requires item(pair, ?p);
    /*@
    ensures
        item(pair, p) &*&
        switch (p) {
            case pair_item(f, s): return item(result, s);
            case key_item(creator, id, isPublicKey, info): return false;
            case data_item(d): return false;
            case encrypted_item(creator, id, info, payload): return false;
        };
    @*/

// A real implementation can simply compare the bitstrings.
bool item_equals(struct item *item1, struct item *item2);
    //@ requires item(item1, ?i1) &*& item(item2, ?i2);
    //@ ensures item(item1, i1) &*& item(item2, i2) &*& result == (i1 == i2);

void item_free(struct item *item);
    //@ requires item(item, _);
    //@ ensures true;

/*@

fixpoint int int_left(int p);
fixpoint int int_right(int p);
fixpoint int int_pair(int f, int s);

lemma_auto void int_left_int_pair(int f, int s);
    requires true;
    ensures int_left(int_pair(f, s)) == f;

lemma_auto void int_right_int_pair(int f, int s);
    requires true;
    ensures int_right(int_pair(f, s)) == s;

fixpoint bool bad(int principal);

// info:
//   int_pair(0, 0): for encryption
//   int_pair(1, server): client nonce
//   int_pair(2, int_pair(client, int_pair(client_nonce_creator, int_pair(client_nonce_public_key, client_nonce_info)))): server nonce

// A definition of "pub" for the example protocol.
fixpoint bool mypub(item i) {
    switch (i) {
        case key_item(o, k, isPublicKey, info): return
            isPublicKey || bad(o) || int_left(info) == 1 && bad(int_right(info)) || int_left(info) == 2 && bad(int_left(int_right(info)));
        case data_item(d): return true;
        case encrypted_item(creator, id, info, m): return
            mypub(m) ||
            switch (m) {
                case key_item(creator0, id0, isPublicKey0, info0): return
                    int_left(info0) == 2 && creator == creator0 && !isPublicKey0 && info == int_pair(0, 0) &&
                    int_left(int_right(info0)) == int_left(int_right(int_right(info0))) &&
                    int_left(int_right(int_right(int_right(info0)))) == 0 &&
                    int_right(int_right(int_right(int_right(info0)))) == int_pair(1, creator);
                case pair_item(f, s): return
                    switch (s) {
                        case key_item(creator0, id0, isPublicKey0, info0): return
                            info0 == int_pair(1, creator) && f == data_item(creator0) && !isPublicKey0 && info == int_pair(0, 0);
                        case pair_item(fs, ss): return
                            switch (ss) {
                                case key_item(creator0, id0, isPublicKey0, info0): return
                                    int_left(info0) == 2 && creator == int_left(int_right(info0)) && !isPublicKey0 && f == data_item(creator0) && info == int_pair(0, 0) &&
                                    (mypub(fs) ||
                                     switch (fs) {
                                         case key_item(creator1, id1, isPublicKey1, info1): return
                                             creator1 == int_left(int_right(int_right(info0))) &&
                                             (isPublicKey1 ? 1 : 0) == int_left(int_right(int_right(int_right(info0)))) &&
                                             info1 == int_right(int_right(int_right(int_right(info0)))) &&
                                             creator1 == creator && !isPublicKey1 && info1 == int_pair(1, creator0);
                                         default: return false;
                                     });
                                default: return false;
                            };
                        default: return false;
                    };
                default: return false;
            };
        case pair_item(f, s): return mypub(f) && mypub(s);
    }
}

@*/

void client(int client, int server, struct item *clientPrivateKey, struct item *serverPublicKey)
    /*@
    requires
        principal(client, _) &*& !bad(client) &*& !bad(server) &*&
        world(mypub) &*&
        key(clientPrivateKey, client, ?cskid, false, int_pair(0, 0)) &*&
        key(serverPublicKey, server, ?spkid, true, int_pair(0, 0));
    @*/
    /*@
    ensures
        principal(client, _) &*&
        world(mypub) &*&
        key(clientPrivateKey, client, cskid, false, int_pair(0, 0)) &*&
        key(serverPublicKey, server, spkid, true, int_pair(0, 0));
    @*/
{
    //@ close keypair_request(int_pair(1, server));
    struct keypair *noncePair = create_keypair();
    struct item *clientNonce = keypair_get_private_key(noncePair);
    //@ open key(clientNonce, _, _, _, _);
    struct item *i0 = keypair_get_public_key(noncePair);
    //@ open key(i0, _, _, _, _);
    item_free(i0);
    
    struct item *i1 = create_data_item(client);
    struct item *i2 = create_pair(i1, clientNonce);
    struct item *i3 = encrypt(serverPublicKey, i2);
    net_send(i3);
    item_free(i1);
    item_free(i2);
    item_free(i3);
    
    struct item *i4 = net_receive();
    struct item *i5 = decrypt(clientPrivateKey, i4);
    struct item *i6 = pair_get_first(i5);
    int s = item_get_data(i6);
    if (s != server) abort();
    struct item *i7 = pair_get_second(i5);
    struct item *i8 = pair_get_first(i7);
    bool eq = item_equals(clientNonce, i8);
    if (!eq) abort();
    struct item *serverNonce = pair_get_second(i7);
    item_free(i4);
    item_free(i5);
    item_free(i6);
    item_free(i7);
    item_free(i8);
    
    //@ assert item(serverNonce, ?sn);
    /*@
    switch (sn) {
        case key_item(creator, id, isPublicKey, info):
        case data_item(d):
        case encrypted_item(creator, id, info, payload):
        case pair_item(fst, snd):
    }
    @*/
    struct item *i9 = encrypt(serverPublicKey, serverNonce);
    net_send(i9);
    item_free(i9);
    
    //@ assert item(clientNonce, ?cn) &*& !mypub(cn) &*& !mypub(sn);
    
    item_free(clientNonce);
    item_free(serverNonce);
}

struct item *get_client_public_key(int client);
    //@ requires true;
    //@ ensures key(result, client, _, true, int_pair(0, 0));

void server(int server, struct item *serverPrivateKey)
    //@ requires principal(server, _) &*& world(mypub) &*& key(serverPrivateKey, server, ?sskid, false, int_pair(0, 0)) &*& !bad(server);
    //@ ensures false;
{
    for (;;)
        //@ invariant principal(server, _) &*& world(mypub) &*& key(serverPrivateKey, server, sskid, false, int_pair(0, 0));
    {
        int client = 0;
        struct item *clientNonce = 0;
        {
            struct item *i1 = net_receive();
            struct item *i2 = decrypt(serverPrivateKey, i1);
            struct item *i3 = pair_get_first(i2);
            client = item_get_data(i3);
            clientNonce = pair_get_second(i2);
            check_is_key(clientNonce);
            item_free(i1);
            item_free(i2);
            item_free(i3);
        }
        //@ open key(clientNonce, ?clientNonceCreator, ?clientNonceId, ?clientNonceIsPublicKey, ?clientNonceInfo);
        
        struct item *serverNonce = 0;
        {
            //@ assert item(clientNonce, ?cn);
            struct item *clientPublicKey = get_client_public_key(client);
            //@ close keypair_request(int_pair(2, int_pair(client, int_pair(clientNonceCreator, int_pair(clientNonceIsPublicKey ? 1 : 0, clientNonceInfo)))));
            struct keypair *noncePair = create_keypair();
            serverNonce = keypair_get_private_key(noncePair);
            //@ open key(serverNonce, _, _, _, _);
            struct item *i0 = keypair_get_public_key(noncePair);
            //@ open key(i0, _, _, _, _);
            struct item *i1 = create_data_item(server);
            struct item *i2 = create_pair(clientNonce, serverNonce);
            struct item *i3 = create_pair(i1, i2);
            struct item *i4 = encrypt(clientPublicKey, i3);
            net_send(i4);
            item_free(i0);
            item_free(i1);
            item_free(i2);
            item_free(i3);
            item_free(i4);
            //@ open key(clientPublicKey, _, _, _, _);
            item_free(clientPublicKey);
        }
        
        {
            struct item *i1 = net_receive();
            struct item *i2 = decrypt(serverPrivateKey, i1);
            bool eq = item_equals(i2, serverNonce);
            if (!eq) abort();
            item_free(i1);
            item_free(i2);
        }
        
        //@ assume(!bad(client));
        //@ assert item(clientNonce, ?nc) &*& item(serverNonce, ?ns);
        /*@
        switch (nc) {
            case key_item(creator, id, isPublicKey, info):
                nc = nc;
                assert creator == client;
                assert info == int_pair(1, server);
            case data_item(d):
                nc = nc;
            case encrypted_item(creator, id, info, payload):
                nc = nc;
            case pair_item(fst, snd):
                nc = nc;
        }
        @*/
        //@ assert !mypub(nc) &*& !mypub(ns);
        item_free(clientNonce);
        item_free(serverNonce);
    }
}

int choose();
    //@ requires true;
    //@ ensures true;

void attacker()
    //@ requires world(mypub) &*& principals(0);
    //@ ensures false;
{
    for (;;)
        //@ invariant world(mypub) &*& principals(?principalCount);
    {
        //@ create_principal(); // Attackers are arbitrary principals.
        for (;;)
            //@ invariant world(mypub) &*& principals(_) &*& principal(?me, ?keyCount);
        {
            int action = choose();
            switch (action) {
                case 0:
                    // Bad principals leak private keys...
                    int info = choose();
                    //@ close keypair_request(info);
                    struct keypair *keypair = create_keypair();
                    struct item *sk = keypair_get_private_key(keypair);
                    struct item *pk = keypair_get_public_key(keypair);
                    //@ open key(sk, _, _, _, _);
                    //@ open key(pk, _, _, _, _);
                    net_send(pk);
                    //@ assume(bad(me));
                    net_send(sk);
                    item_free(sk);
                    item_free(pk);
                    break;
                case 1:
                    // Anyone can publish arbitrary data items...
                    int data = choose();
                    struct item *item = create_data_item(data);
                    net_send(item);
                    item_free(item);
                    break;
                case 2:
                    // Anyone can create pairs of public items...
                    struct item *first = net_receive();
                    struct item *second = net_receive();
                    struct item *pair = create_pair(first, second);
                    item_free(first);
                    item_free(second);
                    net_send(pair);
                    item_free(pair);
                    break;
                case 3:
                    // Anyone can encrypt a public item with a published key...
                    struct item *key = net_receive();
                    struct item *payload = net_receive();
                    check_is_key(key);
                    struct item *item = encrypt(key, payload);
                    //@ open key(key, _, _, _, _);
                    item_free(key);
                    item_free(payload);
                    net_send(item);
                    item_free(item);
                    break;
                case 4:
                    // Anyone can deconstruct a public pair...
                    struct item *pair = net_receive();
                    struct item *first = pair_get_first(pair);
                    struct item *second = pair_get_second(pair);
                    item_free(pair);
                    net_send(first);
                    item_free(first);
                    net_send(second);
                    item_free(second);
                    break;
                case 5:
                    // Anyone can decrypt a public item with a published private key...
                    struct item *key = net_receive();
                    struct item *package = net_receive();
                    check_is_key(key);
                    struct item *payload = decrypt(key, package);
                    //@ open key(key, ?kcreator, ?kid, ?kispublickey, ?kinfo);
                    item_free(key);
                    item_free(package);
                    //@ assert item(payload, ?p);
                    /*@
                    if (mypub(p)) {
                    } else {
                    switch (p) {
                        case key_item(creator, id, isPublicKey, info):
                            p = p;
                        case data_item(d):
                            p = p;
                        case encrypted_item(creator, id, isPublicKey, info):
                            p = p;
                        case pair_item(f, s):
                            p = p;
                            switch (s) {
                                case key_item(creator, id, isPublicKey, info):
                                    p = p;
                                case data_item(d):
                                    p = p;
                                case encrypted_item(creator, id, info, payload0):
                                    p = p;
                                case pair_item(sf, ss):
                                    p = p;
                                    switch (ss) {
                                        case key_item(creator0, id0, isPublicKey0, info0):
                                            p = p;
                                            switch (sf) {
                                                case key_item(creator1, id1, isPublicKey1, info1):
                                                    p = p;
                                                case data_item(d):
                                                    p = p;
                                                case encrypted_item(creator1, id1, info1, payload1):
                                                    p = p;
                                                    assert !kispublickey;
                                                    assert kinfo == int_pair(0, 0);
                                                    assert bad(kcreator) == true;
                                                    if (mypub(sf)) {
                                                        p = p;
                                                    } else {
                                                        p = p;
                                                    }
                                                case pair_item(sff, sfs):
                                                    p = p;
                                            }
                                        case data_item(d):
                                            p = p;
                                        case encrypted_item(creator, id, info, payload0):
                                            p = p;
                                        case pair_item(ssf, sss):
                                            p = p;
                                    }
                            }
                    }
                    }
                    @*/
                    net_send(payload);
                    item_free(payload);
                    break;
            }
        }
        //@ leak principal(_, _);
    }
}
