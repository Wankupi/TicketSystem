<script setup lang="ts">
import { backend_key } from '@/keys';
import { computed } from '@vue/reactivity';
import { ElMessage } from 'element-plus';
import { inject, reactive, ref } from 'vue';
import draggable from "vuedraggable";
import iconMove from '@/components/iconMove.vue';
interface Station {
	name: string,
	stop: number,
	next_time: number,
	next_price: number
};

let trainID = ref("");
let trainType = ref("");
let seatNum = ref(0);
let stations = reactive(new Array<Station>());
let saleDate = ref(["", ""]);
let startTime = ref();

function add_station() {
	stations.push({ name: "", stop: 0, next_price: 0, next_time: 0 });
}
function reset_stations() {
	stations.length = 0;
	add_station();
	add_station();
}
function remove_station(idx: number) {
	stations.splice(idx, 1);
}
reset_stations();


let backend = inject(backend_key)!;

const validate = computed(() => {
	if (!trainID.value || !trainType.value || !startTime.value || !saleDate.value || !seatNum) return false;
	for (let i = 0; i < stations.length; ++i)
		if (!stations[i].name)
			return false;
	return true;
});


function generate_request_command() {
	let cmd = `add_train -i ${trainID.value} -n ${stations.length} -m ${seatNum.value}`;
	cmd += ` -s ${stations.map(e => e.name).join('|')}`;
	cmd += ` -p ${stations.map(e => e.next_price).slice(0, -1).join('|')}`;
	cmd += ` -x ${startTime.value}`;
	cmd += ` -t ${stations.map(e => e.next_time).slice(0, -1).join('|')}`;
	cmd += ` -o ${stations.map(e => e.stop).slice(1, -1).join('|')}`;
	cmd += ` -d ${saleDate.value[0]}|${saleDate.value[1]}`;
	cmd += ` -y ${trainType.value}`;
	return cmd;
}

let history = new Map<string, string>();
function send_request_add_train() {
	if (!validate.value) return;
	let timestamp = (new Date()).toISOString();
	history.set(timestamp, trainID.value);
	backend.send(`[${timestamp}] ${generate_request_command()}`);
}

backend.addEventListener('message', (msg) => {
	let str: string = msg.data;
	let timestamp = str.slice(1, 25);
	if (!history.has(timestamp)) return;
	if (str.charAt(27) == "0") {
		ElMessage.success(`add train ${history.get(timestamp)} success.`);
	}
	else {
		ElMessage.error(`add train ${history.get(timestamp)} failed.`);
	}
});

</script>

<template>
	<h1>添加列车</h1>
	<div>
		<el-input v-model="trainID" minlength="1" maxlength="20" :show-word-limit=true><template
				#prepend>列车ID</template></el-input>
		<el-input v-model="trainType" minlength="1" maxlength="1" :show-word-limit=true><template
				#prepend>列车类型</template></el-input>
		<div><span>座位数</span><el-input-number v-model="seatNum" :min="1" :max="100000" :precision="0"></el-input-number>
		</div>
		<div>
			<span>售票区间</span>
			<el-date-picker v-model="saleDate" type="daterange" range-separator="To" start-placeholder="Start date"
				end-placeholder="End date" :unlink-panels="true" format="MM-DD" value-format="MM-DD" />
		</div>

		<div><span>每日发车时间</span><el-time-picker v-model="startTime" placeholder="Arbitrary time" format="HH:mm"
				value-format="HH:mm" /></div>
		<div>
			总计 {{ stations.length }} 个车站
			<el-button @click="reset_stations">重置车站</el-button>
			<el-button @click="add_station" :disabled="stations.length >= 100">添加车站</el-button>
			<draggable :list="stations" v-bind="{ animation: 200 }" handle=".handle">
				<template #item="{ element, index }">
					<div class="line">
						<el-input v-model="element.name" minlength="1" maxlength="10" :show-word-limit=true
							class="length20"><template #prepend>车站名</template></el-input>
						<div><span>停留时间</span><el-input-number v-model="element.stop" :min="0" :max="10000"
								:precision="0" :disabled="index == 0 || index == stations.length - 1"></el-input-number></div>
						<div><span>站间用时</span><el-input-number v-model="element.next_time" :min="0" :max="10000"
								:precision="0" :disabled="index == stations.length - 1"></el-input-number></div>
						<div><span>站间票价</span><el-input-number v-model="element.next_price" :min="0" :max="100000"
								:precision="0" :disabled="index == stations.length - 1"></el-input-number></div>
						<iconMove class="handle" />
						<el-button @click="remove_station(index)" :disabled="stations.length <= 2">删除此站</el-button>
					</div>
				</template>
			</draggable>
		</div>
		<el-button @click="send_request_add_train" :disabled="!validate">添加</el-button>
	</div>
</template>

<style scoped>
.line {
	display: flex;
	justify-content: space-evenly;
	align-items: center;
}

.length20 {
	max-width: 20em;
}
</style>

