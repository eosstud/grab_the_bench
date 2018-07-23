# EOSSTUD 常见问题的统一回复
## 合约地址：studcontract

Q: EOSPARK查询不到合约？

A: 的确查询不到，技术方面也不清楚为什么，但通过以下命令是能查询合约哈希和ABI的

合约哈希：cleos get code studcontract

合约ABI：cleos get abi studcontract

---

Q: EOS合约可以更改？

A: 这个的确，我们也在想解决办法。现在我们公布了源代码，有能力的可以验证并且随时查看链上哈希。当然万一团队有违规行为，可以联系bp冻结账号，相信短时间那么大规模，bp不会坐视不管的。不过话说回来，不是智能合约的话分红提现可以秒到账？

---

Q: 提现不成功？提现记录看不到？

A: 转账记录看不到的，直接加在余额里面的。

不知为何现在钱包显示不了智能合约的自动转账记录。

区块没有记录是区块浏览器和钱包的锅。。。

我们不背。。。

你发出0.0001的同时就到了，是同一时间处理的。

---

Q: 转账提示错误

A: 请确保账户有充足的CPU和带宽资源。如果转账依旧不成功请考虑更换钱包。EOS转账指令一般有默认的CPU和带宽资源限制，在网络繁忙时容易出错，一般解决方法是稍候再试。如果使用cleos命令行转账可以使用--max-cpu-usage-ms 以及--max-net-usage来增加转账成功率。

---

Q: 网站进不去

A: 网站使用了cloudflare服务，有时候会因线路问题导致网站访问不正常。网站现阶段还有很多用户体验方面的问题需要优化，团队正在加班加点改进。

---

网站只是作为引导，智能合约的交互能通过转账直接进行。

所有用户数据能通过以下命令查询

---

查询当前状态：cleos get table studcontract studcontract counter

查询当前玩家列表： cleos get table studcontract studcontract playertable

查询历史分红状态：cleos get table studcontract studcontract userbalance
