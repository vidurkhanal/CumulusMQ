<h1>CumulusMQ</h1>

CumulusMQ is a cloud-native message queue that offers persistent storage capabilities by seamlessly integrating with AWS S3 and Cloudflare R2 object storage services. It is designed to handle large volumes of messages with high throughput, ensuring reliable delivery and durability.

<h3>CURRENTLY WIP [[MAY INTRODUCE BREAKING CHANGES.]]</h3>

Features

<li> <b>Cloud-Native:</b> NimbusQueue is built from the ground up to take advantage of cloud infrastructure, providing high availability and scalability.</li>
<li> <b>Persistent Storage:</b> Messages are persisted to AWS S3 and Cloudflare R2 object storage, ensuring data durability and resilience against node failures.</li>
<li> <b>High Performance:</b> NimbusQueue is optimized for low latency and high throughput, making it suitable for demanding real-time applications.</li>
<li> <b>Horizontal Scalability:</b> NimbusQueue can scale horizontally by adding more nodes to the cluster, allowing it to handle increasing message volumes seamlessly.</li>
<li> <b>Flexible Delivery Guarantees:</b> Choose between at-least-once, at-most-once, or exactly-once delivery semantics based on your application's requirements. </li>
<li> <b>Pub/Sub and Queue Patterns:</b> Supports both publish/subscribe and traditional queue messaging patterns. </li>

Protocol

| Message Length | Message Type | Body Length | Message Body |  
|----------------|--------------|-------------|--------------|
| 4 bytes        | 1 byte       | 4 bytes     | n bytes      |


| Message Type Value | Action      |
|--------------------|-------------|
| 0x01               | Subscribe   |
| 0x02               | Unsubscribe |
| 0x03               | Publish     |
| 0x04               | Retrieve    |



