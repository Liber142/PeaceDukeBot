#!/bin/sh
set -eu

FILE="${VLESS_FILE:-/data/vless.txt}"
LISTEN_PORT="${LISTEN_PORT:-2080}"

[ -f "$FILE" ] || { echo "proxy: file $FILE not found" >&2; exit 1; }
LINK=$(grep -m1 '^vless://' "$FILE" | tr -d '\r ' || true)
[ -n "$LINK" ] || { echo "proxy: no vless:// link in $FILE" >&2; exit 1; }

REST=${LINK#vless://}
REST=${REST%%#*}
UUID=${REST%%@*}
AFTER=${REST#*@}
case "$AFTER" in
  *\?*) HOSTPORT=${AFTER%%\?*}; QUERY=${AFTER#*\?} ;;
  *)    HOSTPORT=$AFTER;        QUERY="" ;;
esac
HOST=${HOSTPORT%:*}
PORT=${HOSTPORT##*:}

q() { printf '%s' "$QUERY" | tr '&' '\n' | sed -n "s/^$1=//p" | head -n1; }

TYPE=$(q type);   TYPE=${TYPE:-tcp}
SEC=$(q security)
SNI=$(q sni);     SNI=${SNI:-$HOST}
FP=$(q fp);       FP=${FP:-chrome}
PBK=$(q pbk)
SID=$(q sid)
ALPN=$(q alpn)
SVC=$(q serviceName)
FLOW=$(q flow)

ALPN_JSON=""
if [ -n "$ALPN" ]; then
  ALPN_JSON=$(printf '%s' "$ALPN" | sed 's/%2[Cc]/,/g; s/,/","/g; s/^/,"alpn":["/; s/$/"]/')
fi

case "$SEC" in
  reality)
    TLS=$(printf '"tls":{"enabled":true,"server_name":"%s"%s,"utls":{"enabled":true,"fingerprint":"%s"},"reality":{"enabled":true,"public_key":"%s","short_id":"%s"}}' \
      "$SNI" "$ALPN_JSON" "$FP" "$PBK" "$SID") ;;
  tls)
    TLS=$(printf '"tls":{"enabled":true,"server_name":"%s"%s,"utls":{"enabled":true,"fingerprint":"%s"}}' \
      "$SNI" "$ALPN_JSON" "$FP") ;;
  *)
    TLS='"tls":{"enabled":false}' ;;
esac

case "$TYPE" in
  grpc) TRANSPORT=$(printf ',"transport":{"type":"grpc","service_name":"%s"}' "$SVC") ;;
  ws)   WSPATH=$(q path | sed 's/%2[Ff]/\//g')
        TRANSPORT=$(printf ',"transport":{"type":"ws","path":"%s"}' "${WSPATH:-/}") ;;
  tcp)  TRANSPORT="" ;;
  *)    echo "proxy: unsupported transport type: $TYPE" >&2; exit 1 ;;
esac

FLOW_JSON=""
if [ -n "$FLOW" ]; then FLOW_JSON=",\"flow\":\"$FLOW\""; fi

cat > /tmp/config.json <<EOF
{
  "log": { "level": "info" },
  "inbounds": [{
    "type": "tun", "tag": "tun-in", "interface_name": "tun0",
    "address": ["172.19.0.1/30"],
    "mtu": 1400,
    "auto_route": true
  }],
  "outbounds": [
    {
      "type": "vless", "tag": "proxy",
      "server": "${HOST}", "server_port": ${PORT},
      "uuid": "${UUID}", "packet_encoding": "xudp"${FLOW_JSON},
      ${TLS}${TRANSPORT}
    },
    { "type": "direct", "tag": "direct" }
  ],
  "route": {
    "auto_detect_interface": true,
    "rules": [
      { "action": "sniff" },
      { "protocol": "dns", "action": "hijack-dns" },
      { "ip_is_private": true, "outbound": "direct" }
    ],
    "final": "proxy"
  }
}
EOF

echo "proxy: config generated for ${HOST}:${PORT} (${TYPE}, ${SEC:-none})"
sing-box check -c /tmp/config.json
exec sing-box -c /tmp/config.json run
