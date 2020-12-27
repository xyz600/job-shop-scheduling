# job-shop-scheduling

## solver

GT-Algorithm を用いて、スケジュールを算出し、visualizer  で確認するための問題と解を json で出力します。  
問題と解の json format は以下のようになっています。

### 問題

```json5
{
  "job_size": 100,
  "machine_size": 20,
  "operation_table": [
      [
        // job 0 の operation_id
      ],
      [
        // job 1 の operation_id
      ],
      // ...
      [
        // job 99 の operation_id
      ]
  ],
  "operations": [
    {
      "id": 0,
      "job": 0,
      "machine": 12,
      "time": 83
    },
    // ...
    {
      "id": 1999,
      "job": 99,
      "machine": 19,
      "time": 60
    }
  ]
}
```

* job_size
    * job 数
* machine_size
    * job をこなす機械の数
* operation_table
    * job を構成する operation の順番
* operations
    * job を構成する operation の情報
        * id: operation id
        * job: 属する job 番号
        * machine: 使用する machine id
        * time: operation の実行必要な時間

### 解

```json5
{
  "process_list": [
    [
      // machine 0 でこなす operation の情報
      {
        "operation_id": 20,
        "start_time": 0
      },
      // ...
    ],
    [
      // machine 1 でこなす operation の情報
      {
        "operation_id": 1040,
        "start_time": 0
      },
      // ...
    ]
    // ...
  ]
}
```

* process_list
    * 各機械でどの operation をいつ実行するかを示す

## [visualizer](https://xyz600.github.io/job-shop-scheduling/)

上記の solver で生成した json を与えることで、スケジュールが可視化される

* 直前が空白になっている、非効率と考えられる operation に対して、(job_id, operation_id) を hover させる
* 特定 job の operation を別色で表示
* 中身の json を確認する debug 用表示