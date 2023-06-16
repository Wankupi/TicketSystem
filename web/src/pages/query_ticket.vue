<script setup lang="ts">
import { backend_key } from '@/keys';
import type { Ticket } from '@/keys';
import { computed } from '@vue/reactivity';
import { ElMessage } from 'element-plus';
import { inject, reactive, ref } from 'vue';
import ticket from '@/components/ticket.vue';

let backend = inject(backend_key)!;

let from = ref("");
let to = ref("");
let queryDate = ref("");
let sortby = ref("time");
const validate = computed(() => {
	return from.value && to.value && queryDate.value && sortby.value;
});

let history = new Set<string>();
let status = ref(0);

let query_result = reactive(new Array<Ticket>());
function send_request_query_ticket(method: string) {
	if (!validate.value) return;
	status.value = 3;
	let timestamp = (new Date()).toISOString();
	history.add(timestamp);
	backend.send(`[${timestamp}] query_${method} -s ${from.value} -t ${to.value} -d ${queryDate.value} -p ${sortby.value}`);
}
backend.addEventListener('message', (msg) => {
	let str: string = msg.data;
	let timestamp = str.slice(1, 25);
	if (!history.has(timestamp)) return;
	if (str.charAt(27) == "-" && str.charAt(28) == "1") {
		status.value = 2;
		ElMessage.error(`query failed.`);
	}
	else {
		parse_tickets(str.slice(27));
		status.value = 1;
		ElMessage.success(`query success.`);
	}
});


function parse_tickets(str: string) {
	let lines = str.split("\n");
	query_result.length = 0;
	for (let i = 1; i < lines.length; ++i) {
		let eles = lines[i].split(" ");
		query_result.push(
			{
				trainID: eles[0],
				from: eles[1],
				leave: eles[2] + " " + eles[3],
				to: eles[5],
				arrive: eles[6] + " " + eles[7],
				price: parseInt(eles[8]),
				seat: parseInt(eles[9])
			}
		);
	}
}

</script>


<template>
	<h3>查询车票</h3>
	<div class="center">
		<el-input v-model="from" minlength="1" maxlength="10" :show-word-limit=true><template
				#prepend>始发站</template></el-input>
		<el-input v-model="to" minlength="1" maxlength="10" :show-word-limit=true><template
				#prepend>终点站</template></el-input>
		<el-date-picker v-model="queryDate" format="MM-DD" value-format="MM-DD" />
		<el-select v-model="sortby">
			<el-option key="time" value="time"></el-option>
			<el-option key="cost" value="cost"></el-option>
		</el-select>
		<el-button @click="send_request_query_ticket('ticket')">查询直达车票</el-button>
		<el-button @click="send_request_query_ticket('transfer')">查询换乘车票</el-button>
		<div v-show="status == 1">
			<div v-show="query_result.length == 0">无符合条件的列车</div>
			<el-space wrap direction="vertical">
				<ticket v-for="(train, index) in query_result" :train_info="train" @click=""></ticket>
			</el-space>
		</div>
	</div>
</template>
