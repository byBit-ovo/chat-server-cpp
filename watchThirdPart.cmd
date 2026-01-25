查看etcd已注册的服务:
ETCDCTL_API=3 etcdctl --endpoints=http://127.0.0.1:2379 get /service --prefix   

登录redis:
redis-cli 


GET /user/_doc/_search
{
  "query": {
    "match_all": {}
  }
}