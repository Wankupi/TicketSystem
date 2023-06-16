<script setup lang="ts">
import { backend_key, current_user_key } from '@/keys';
import { ElMessage } from 'element-plus';
import { inject, reactive, ref } from 'vue';

let current_user = inject(current_user_key)!;
let backend = inject(backend_key)!;
let history = new Map<string, string>();

interface bill_info {
	status: string,
	trainID: string,
	leave: string,
	from: string,
	to: string,
	arrive: string,
	price: string,
	count: string
}

let query_result = reactive(new Array<bill_info>());

function send_request_query_order() {
	if (!current_user.value) return;
	let timestamp = (new Date()).toISOString();
	history.set(timestamp, "query");
	backend.send(`[${timestamp}] query_order -u ${current_user.value}`);
}

backend.addEventListener('message', (msg) => {
	let str: string = msg.data;
	let timestamp = str.slice(1, 25);
	if (!history.has(timestamp)) return;
	let op = history.get(timestamp)!;
	if (str.charAt(27) == "-" && str.charAt(28) == "1") {
		ElMessage.error(`${op == "query" ? "查询" : "退票"}失败`);
	}
	else {
		if (op == "query")
			parse_bills(str.slice(27));
		else {
			let index = parseInt(op.split(' ')[1]);
			// query_result.splice(index, 1);
			query_result[index].status = "[refunded]";
		}
		ElMessage.success(`${op == "query" ? "查询" : "退票"}成功`);
	}
});

function parse_bills(str: string) {
	query_result.length = 0;
	str.split('\n').forEach((val, index) => {
		if (index == 0) return;
		let a = val.split(' ');
		query_result.push({
			status: a[0],
			trainID: a[1],
			from: a[2],
			leave: a[3] + ' ' + a[4],
			to: a[6],
			arrive: a[7] + ' ' + a[8],
			price: a[9],
			count: a[10]
		});
	});
}

function send_request_refund(index: number) {
	if (!current_user.value) return;
	let timestamp = (new Date()).toISOString();
	history.set(timestamp, `refund ${index}`);
	backend.send(`[${timestamp}] refund_ticket -u ${current_user.value} -n ${index + 1}`);
}

</script>
<template>
	<h3>查询订单信息</h3>
	<el-button @click="send_request_query_order" :disabled="!current_user">查询</el-button>
	<el-table :data="query_result">
		<el-table-column label="状态">
			<template #default="scope">
				<el-text :type='scope.row.status == "[success]" ? "success" : (scope.row.status == "[pending]" ? "warning" : "info")'>
					{{ scope.row.status == "[success]" ? "成功" : (scope.row.status == "[pending]" ? "候补" : "已退") }}
				</el-text>
			</template>
		</el-table-column>
		<el-table-column prop="from" label="起点"></el-table-column>
		<el-table-column prop="leave" label="离站时间"></el-table-column>
		<el-table-column prop="to" label="终点"></el-table-column>
		<el-table-column prop="arrive" label="到站时间"></el-table-column>
		<el-table-column prop="price" label="票价"></el-table-column>
		<el-table-column prop="count" label="数量"></el-table-column>
		<el-table-column label="操作">
			<template #default="scope">
				<el-button :type='scope.row.status == "[pending]" ? "warning" : "danger"'
					@click="send_request_refund(scope.$index)" v-if='scope.row.status != "[refunded]"' plain>
					{{ scope.row.status == "[pending]" ? "取消" : "退票" }}
				</el-button>
			</template>
		</el-table-column>
	</el-table>
</template>