<script setup lang="ts">
import { ElMessage } from "element-plus";
import iconPrice from "./iconPrice.vue";
import iconSeat from "./iconSeat.vue";
import type { Ticket } from "@/keys";
import { bill_info_key, pageState_key, PageState, current_user_key } from "@/keys";
import { inject } from "vue";

let props = defineProps<{ train_info: Ticket }>();
let bill = inject(bill_info_key)!;
let pageState = inject(pageState_key)!;
let current_user = inject(current_user_key)!;

function get_time(leave: string, arrive: string) {
	let a = new Date("2023-" + leave), b = new Date("2023-" + arrive);
	let mins = (b.valueOf() - a.valueOf()) / (1000 * 60);
	if (mins > 60) return `${Math.floor(mins / 60)}小时${mins % 60}分钟`;
	else return `${mins}分钟`;
}


function buy_this() {
	if (!current_user.value) return;
	bill.trainID = props.train_info.trainID;
	bill.from = props.train_info.from;
	bill.to = props.train_info.to;
	bill.leave = props.train_info.leave.slice(0, 5);
	bill.seat = 1;
	pageState.value = PageState.buy_ticket;
}

</script>

<template>
	<el-card class="train-card" @click="buy_this">
		<div class="col">
			<div class="row2">
				<el-text class="mx-1" type="primary" size="large">{{ train_info.leave }}</el-text>
				<el-text class="mx-1">{{ train_info.from }}</el-text>
			</div>
			<div class="row2">
				<el-text class="mx-1" type="success" tag="b" size="large">{{ train_info.trainID }}</el-text>
				<el-text class="mx-1" type="primary">{{ get_time(train_info.leave, train_info.arrive) }}</el-text>
			</div>
			<div class="row2">
				<el-text class="mx-1" type="primary" size="large">{{ train_info.arrive }}</el-text>
				<el-text class="mx-1">{{ train_info.to }}</el-text>
			</div>
		</div>
		<div class="col">
			<span>
				<iconPrice /><el-text class="mx-1">{{ train_info.price }}</el-text>
			</span>
			<span>
				<iconSeat /><el-text class="mx-1">{{ train_info.seat }}</el-text>
			</span>
		</div>
	</el-card>
</template>

<style scoped>
.row2 {
	display: grid;
	text-align: center;
}

.col {
	display: flex;
	justify-content: space-around;
}

.train-card {
	width: 25em;
}
</style>