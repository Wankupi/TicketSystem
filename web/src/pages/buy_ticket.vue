<script setup lang="ts">
import { bill_info_key, type Bill, current_user_key, backend_key } from '@/keys';
import { computed } from '@vue/reactivity';
import { ElMessage } from 'element-plus';
import { inject, ref } from 'vue';

let bill = inject(bill_info_key)!;
let current_user = inject(current_user_key)!;
let backend = inject(backend_key)!;
let accept_queue = ref(false);

const validate = computed(() => {
	if (!bill.trainID || !bill.from || !bill.leave || !bill.to)
		return false;
	if (!current_user.value)
		return false;
	return true;
});

let history = new Set<string>();
function send_request_buy_ticket() {
	if (!validate.value) return;
	let timestamp = (new Date()).toISOString();
	history.add(timestamp);
	backend.send(`[${timestamp}] buy_ticket -u ${current_user.value} -i ${bill.trainID} -d ${bill.leave} -n ${bill.seat} -f ${bill.from} -t ${bill.to} -q ${accept_queue.value ? "true" : "false"}`);
}

backend.addEventListener('message', (msg) => {
	let str: string = msg.data;
	let timestamp = str.slice(1, 25);
	if (!history.has(timestamp)) return;
	if (str.charAt(27) == "-" && str.charAt(28) == "1")
		ElMessage.error(`buy ticket failed.`);
	else if (str.charAt(27) == "q")
		ElMessage.info("已进入候补队列");
	else
		ElMessage.success(`购票成功，总价 ${str.slice(27)} 元。`);
});
</script>

<template>
	<h3>购买车票</h3>
	<div class="center">
		<el-input v-model="bill.trainID" minlength="1" maxlength="20" :show-word-limit=true><template
				#prepend>列车ID</template></el-input>
		<el-input v-model="bill.from" minlength="1" maxlength="10" :show-word-limit=true><template
				#prepend>出发</template></el-input>
		<el-input v-model="bill.to" minlength="1" maxlength="10" :show-word-limit=true><template
				#prepend>到达</template></el-input>
		<el-date-picker v-model="bill.leave" format="MM-DD" value-format="MM-DD" />
		<span>数量</span><el-input-number v-model="bill.seat" :min="1" :max="100000" :precision="0"></el-input-number>
		<el-switch v-model="accept_queue" active-text="接受候补" inactive-text="不接受候补"></el-switch>
		<el-button @click="send_request_buy_ticket" :disabled="!validate">购买</el-button>
	</div>
</template>
