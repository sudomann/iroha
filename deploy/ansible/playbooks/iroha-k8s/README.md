# To use:
```
ansible-playbook playbooks/iroha-k8s/main.yml

```
* Tested to work on Google Kubernetes Engine


# To destroy the k8s environment created by iroha:

## If you went with the default namespace
```
kubectl delete -f path/to/iroha-k8s-peer-keys.yml \
               -f path/to/iroha-k8s-services.yml \
               -f path/to/iroha-k8s-configmap.yml \
               -f path/to/iroha-k8s.yml
```

## If you deployed with a custom namespace
```
kubectl delete namespace <namespace>
kubectl delete service <iroha_pod_basename>
```
e.g. based on default config
```
kubectl delete namespace iroha-network
kubectl delete service iroha-peer
```

Convenient way to restart all pods in a deployment/statefulset

> *grahamdumpleton* in k8s community slack:
  "Update an annotation in the pod template spec part":
```
$ kubectl patch statefulset/iroha-peer --patch \
   "{\"spec\":{\"template\":{\"metadata\":{\"annotations\":{\"last-restart\":\"`date +'%s'`\"}}}}}"
```