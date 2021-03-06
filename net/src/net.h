/*
 * AVSystem Commons Library
 *
 * Copyright (C) 2014 AVSystem <http://www.avsystem.com/>
 *
 * This code is free and open source software licensed under the MIT License.
 * See the LICENSE file for details.
 */

#ifndef NET_H
#define NET_H

#include <stdint.h>

#include <avsystem/commons/socket_v_table.h>

#define MODULE_NAME avs_net
#include <x_log_config.h>

#ifdef HAVE_VISIBILITY
#pragma GCC visibility push(hidden)
#endif

/**
 * Note: the _actual_ maximum hostname length is not precisely defined.
 * NI_MAXHOST on Linux is actually a very generous 1025 (incl. nullbyte). DNS
 * frame format allows for up to 253 (excl. nullbyte), and also each segment
 * (substring between the dots) may be at most 64 characters long. Maximum
 * length of a TLS certificate's CN is 64 (excl. nullbyte), but it may contain
 * wildcards (even though we don't support them here in Commons).
 *
 * So... let's use 256 ;)
 */
#define NET_MAX_HOSTNAME_SIZE 256

#define NET_PORT_SIZE 6

#define AVS_NET_RESOLVE_DUMMY_PORT "1337"

int _avs_net_create_tcp_socket(avs_net_abstract_socket_t **socket,
                               const void *socket_configuration);
int _avs_net_create_udp_socket(avs_net_abstract_socket_t **socket,
                               const void *socket_configuration);

#ifdef WITH_SSL
int _avs_net_create_ssl_socket(avs_net_abstract_socket_t **socket,
                               const void *socket_configuration);
int _avs_net_create_dtls_socket(avs_net_abstract_socket_t **socket,
                               const void *socket_configuration);
#endif

int _avs_net_get_af(avs_net_af_t addr_family);

int _avs_net_get_socket_type(avs_net_socket_type_t socket_type);

#ifdef HAVE_VISIBILITY
#pragma GCC visibility pop
#endif

#endif /* NET_H */

