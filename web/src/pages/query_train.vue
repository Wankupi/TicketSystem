<script setup lang="ts">
import { backend_key } from '@/keys';
import { computed } from '@vue/reactivity';
import { ElMessage } from 'element-plus';
import { inject, reactive, ref } from 'vue';

let backend = inject(backend_key)!;
let trainID = ref("");
let queryDate = ref("");

interface StationInfo {
	name: string,
	arrive: string,
	leave: string,
	total_time: string,
	next_seat: string
};

interface TrainInfo {
	trainID: string,
	trainType: string,
	stations: Array<StationInfo>;
};

let query_result: TrainInfo = reactive({ str: '', trainID: '', trainType: '', stations: [] });
const validate = computed(() => { return Boolean(trainID.value); })

let history = new Map<string, string[]>();

function send_request_train_re_de(method: string) {
	if (!validate.value) return;
	let timestamp = (new Date()).toISOString();
	history.set(timestamp, [trainID.value, method]);
	backend.send(`[${timestamp}] ${method}_train -i ${trainID.value}`);
}

function send_request_query_train() {
	if (!validate.value) return;
	let timestamp = (new Date()).toISOString();
	history.set(timestamp, [trainID.value, "query"]);
	backend.send(`[${timestamp}] query_train -i ${trainID.value} -d ${queryDate.value}`);
}

backend.addEventListener('message', (msg) => {
	let str: string = msg.data;
	let timestamp = str.slice(1, 25);
	if (!history.has(timestamp)) return;
	let action = history.get(timestamp)!;
	if (str.charAt(27) == "-" && str.charAt(28) == "1")
		ElMessage.error(`${action[1]} train ${action[0]} failed.`);
	else {
		if (action[1] == "query")
			parse_result(str.slice(27));
		ElMessage.success(`${action[1]} train ${action[0]} success.`);
	}
});

function parse_result(data: string) {
	let a = data.split('\n');
	[query_result.trainID, query_result.trainType] = a[0].split(' ');
	a.splice(0, 1);
	let stations: Array<StationInfo> = [];
	a.forEach(line => {
		let b = line.split(" ");
		stations.push({ name: b[0], arrive: b[1] + ' ' + b[2], leave: b[4] + ' ' + b[5], total_time: b[6], next_seat: b[7] });
	});
	query_result.stations = stations;
}
</script>

<template>
	<h1>列车信息</h1>
	<div>
		<el-input v-model="trainID" minlength="1" maxlength="20" :show-word-limit=true><template
				#prepend>列车ID</template></el-input>
		<el-button @click="send_request_train_re_de('release')" :disabled="!validate">发布</el-button>
		<el-button @click="send_request_train_re_de('delete')" :disabled="!validate">删除</el-button>
		<el-date-picker v-model="queryDate" format="MM-DD" value-format="MM-DD" />
		<el-button @click="send_request_query_train" :disabled="!validate || !queryDate">查询该日车票</el-button>
	</div>
	<div>
		<el-table :data="query_result.stations" style="width: 100%">
			<el-table-column prop="name" label="停靠站" />
			<el-table-column prop="arrive" label="到点" />
			<el-table-column prop="leave" label="开点" />
			<el-table-column prop="total_time" label="历时" />
			<el-table-column prop="next_seat" label="余座" />
		</el-table>
	</div>
</template>
